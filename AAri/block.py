from __future__ import annotations

import dataclasses
from collections import OrderedDict
from typing import List, Dict, Any, Union, Type

import numpy as np

import AAri_cpp  # Import the Pybind11 module
from AAri_cpp import Entity

"""
TODO - A lot of this is deprecated post switch to entt backend, and needs to be rewritten
"""


def dB(db_val: float) -> float:
    """
    Convert a decibel value to a linear value
    :param db_val:
    :return:
    """
    return 10 ** (db_val / 20)


@dataclasses.dataclass(frozen=True, eq=True)
class ParamDef:
    name: str
    io_type: Type[AAri_cpp.InputOutput]


class Param:
    def __init__(
        self,
        name: str,
        io_number: int,
        io_type: Type[AAri_cpp.InputOutput],
        is_input: bool,
    ):
        self.name = name
        self.io_number = io_number
        self.io_type = io_type
        self.is_input = is_input

    @property
    def width(self) -> int:
        match self.io_type:
            case AAri_cpp.Input1D, AAri_cpp.Output1D:
                return 1
            case AAri_cpp.OutputND2, AAri_cpp.InputND2:
                return 2
            case AAri_cpp.InputND4:
                return 4
            case AAri_cpp.InputND8:
                return 8
            case AAri_cpp.InputND16:
                return 16
            case AAri_cpp.InputND32:
                return 32
            case other:
                raise ValueError(f"Invalid io_type {self.io_type}")


class ParamExpression:
    def connect(self, input_param: "AttachedParam"):
        pass


class AttachedParam(ParamExpression):
    def __init__(self, block: "Block", param: Param):
        self.block = block
        self.param = param

    @property
    def value(self) -> Union[float, np.ndarray]:
        return self.block.view_inputs_outputs()[self.entity]

    @property
    def entity(self) -> Entity:
        if self.param.is_input:
            return self.block.cpp_block.input_ids[self.param.io_number]
        else:
            return self.block.cpp_block.output_ids[self.param.io_number]

    def __add__(self, other: Union[float, int, "AttachedParam", "ScaledParam"]):
        match other:
            case float(other):
                return ScaledParam(self, 1.0, other)
            case int(other):
                return ScaledParam(self, 1.0, float(other))
            case AttachedParam(other):
                if self.param.width != other.param.width:
                    raise RuntimeError("Cannot add parameters with different widths")
                return ScaledParam(self, 1.0, 0.0) + ScaledParam(other, 1.0, 0.0)
            case ScaledParam(other):
                if self.param.width != other.attached_param.param.width:
                    raise RuntimeError("Cannot add parameters with different widths")
                return ScaledParam(self, 1.0, 0.0) + other

    def __radd__(self, other):
        return self + other

    def __mul__(self, other: float | int):
        return ScaledParam(self, float(other), 0.0)

    def __rmul__(self, other: float):
        return self * other

    def connect(self, input_param: "AttachedParam"):
        return ScaledParam(self, 1.0, 0.0).connect(input_param)


class ScaledParam(ParamExpression):
    def __init__(self, attached_param: AttachedParam, gain: float, offset: float):
        self.attached_param = attached_param
        self.gain = gain
        self.offset = offset

    def __add__(self, other: Union[float, int, AttachedParam, "ScaledParam"]):
        match other:
            case float(other):
                return ScaledParam(self.attached_param, self.gain, self.offset + other)
            case int(other):
                return ScaledParam(
                    self.attached_param, self.gain, self.offset + float(other)
                )
            case AttachedParam(other):
                if self.attached_param.param.width != other.param.width:
                    raise RuntimeError("Cannot add parameters with different widths")
                return AddedParams(self, ScaledParam(other, 1.0, 0.0))
            case ScaledParam(other):
                if self.attached_param.param.width != other.attached_param.param.width:
                    raise RuntimeError("Cannot add parameters with different widths")
                return AddedParams(self, other)

    def __radd__(self, other):
        return self + other

    def __mul__(self, other: float | int):
        return ScaledParam(
            self.attached_param, self.gain * float(other), self.offset * float(other)
        )

    def __rmul__(self, other: float):
        return self * other

    def connect(self, input_param: AttachedParam):
        """
        Connect this output parameter to another input parameter
        :param input_param:
        :return:
        """
        if self.attached_param.param.is_input:
            raise RuntimeError("Cannot connect inputs to inputs")
        if not input_param.param.is_input:
            raise RuntimeError("Cannot connect to outputs")

        if input_param.param.width != self.attached_param.param.width:
            raise RuntimeError(
                "Cannot connect block parameters with different input and output size"
            )
        self.attached_param.block.engine.add_wire(
            self.attached_param, input_param, self.gain, self.offset
        )


class AddedParams(ParamExpression):
    def __init__(
        self, param1: ScaledParam | AddedParams, param2: ScaledParam | AddedParams
    ):
        self.params_list = []
        if isinstance(param1, ScaledParam):
            self.params_list.append(param1)
        else:
            self.params_list += param1.params_list
        if isinstance(param2, ScaledParam):
            self.params_list.append(param2)
        else:
            self.params_list += param2.params_list

    def connect(self, target_input: AttachedParam | MixerBlock):
        """
        First create a mono or stereo mixer after checking all the blocks outputs are of width 1 or 2,
        and then connect that to the input_param
        :param target_input:
        :return:
        """
        # Check all the blocks are of the same width =  1 or 2
        engine = self.params_list[0].attached_param.block.engine
        width = self.params_list[0].attached_param.param.width
        for param in self.params_list:
            if param.attached_param.param.width != width:
                raise RuntimeError(
                    "Cannot connect parameters with different widths to a single input"
                )
        if width > 2:
            raise RuntimeError("Only widths of 1 or 2 are suppported so far")
        target_width = (
            target_input.param.width
            if isinstance(target_input, AttachedParam)
            else (2 if isinstance(target_input, StereoMixer) else 1)
        )
        # First mix the list of params
        mixer_size = 2 ** int(np.ceil(np.log2(len(self.params_list))))
        mixer = MonoMixer(mixer_size) if width == 1 else StereoMixer(mixer_size)
        for p in self.params_list:
            engine.add_wire(p.attached_param, mixer, gain=p.gain, offset=p.offset)
        # Then connect the mixer to the input
        engine.add_wire(mixer.out, target_input)


class MultipliedParams(ParamExpression):
    def __int__(
        self,
        param1: ScaledParam | "MultipliedParams",
        param2: ScaledParam | "MultipliedParams",
    ):
        self.params1 = param1
        self.params2 = param2

    def connect(self, input_param: "AttachedParam"):
        """
        First create a Product block then connect it to the input_param
        :param input_param:
        :return:
        """
        # TODO
        raise NotImplementedError()


class BlockWithParametersMeta(type):
    def __new__(cls, name, bases, dct):
        params = cls.create_params(
            dct.get("INPUTS", {}),
            True,
        )
        params |= cls.create_params(
            dct.get("OUTPUTS", {}),
            False,
        )
        for name, parameter in params.items():
            # Define getter and setter
            def getter(self: "Block", p=parameter):
                return AttachedParam(self, p)

            # Define setter
            def setter(
                self: "Block",
                value: Union[float, np.ndarray, AttachedParam],
                p=parameter,
            ):
                """
                Set the value of a parameter to either a numerical
                value or the output parameter of a block
                :param self:
                :param value:
                :return:
                """
                if parameter.is_input:
                    a = AttachedParam(self, p)
                    if isinstance(value, ParamExpression):
                        value.connect(a)
                    else:
                        match p.width:
                            case 1:
                                self.engine.engine.set_input_1d(
                                    a.entity,
                                    value,
                                )
                            case 2:
                                self.engine.engine.set_input_2d(
                                    a.entity,
                                    value,
                                )
                            case 4:
                                self.engine.engine.set_input_4d(
                                    a.entity,
                                    value,
                                )
                            case other:
                                raise RuntimeError(
                                    f"Invalid width for parameter {name}: {other}"
                                )
                else:
                    raise RuntimeError("Cannot set output")

            # Create and set property
            prop = property(getter, setter)
            dct[name] = prop

        dct["_all_params"] = params

        return super(BlockWithParametersMeta, cls).__new__(cls, name, bases, dct)

    @staticmethod
    def create_params(
        params: List[ParamDef],
        is_input: bool,
    ) -> Dict[str, Param]:
        params_dict = OrderedDict()
        for i, param in enumerate(params):
            params_dict[param.name] = Param(
                param.name,
                i,
                param.io_type,
                is_input,
            )
        return params_dict


class Block(metaclass=BlockWithParametersMeta):
    def __init__(self, entity: Entity):
        from AAri.audio_engine import AudioEngine  # Avoid circular import

        self.entity = entity
        self.engine = AudioEngine()  # Get the unique instance
        self.cpp_block = self.engine.engine.view_block(entity)
        self.engine.blocks.add(self)

    def __del__(self):
        self.engine.blocks.remove(self)

    @property
    def input_ids(self) -> List[Entity]:
        return self.cpp_block.inputIds

    @property
    def output_ids(self) -> List[Entity]:
        return self.cpp_block.outputIds

    def view_inputs_outputs(self) -> Dict[Entity, Any]:
        return self.engine.engine.view_block_io(self.entity)


class MixerBlock(Block):
    def __init__(self, entity: Entity, num_inputs: int):
        super().__init__(entity)
        self.num_inputs = num_inputs
        # TODO add "output" parameter

    def find_free_slot(self) -> int:
        wires = self.engine.get_wires_to_block(self)
        used_inputs = np.zeros(self.num_inputs)
        # Find the free inputs
        for wire in wires:
            used_inputs[wire.to_input] = 1
        free_slots = np.where(used_inputs == 0)
        if len(free_slots[0]) == 0:
            raise RuntimeError("No free inputs")
        return int(free_slots[0][0])


class StereoMixer(MixerBlock):
    OUTPUTS = [ParamDef("out", AAri_cpp.OutputND2)]

    def __init__(self, size: int = 4):
        from AAri.audio_engine import AudioEngine  # Avoid circular import

        engine = AudioEngine()
        match size:
            case (2):
                entity = AAri_cpp.StereoMixer2.create(engine.engine)
                self.transmit_mono_func = AAri_cpp.Wire.transmit_mono_to_stereo_mixer_2
                self.transmit_stereo_func = (
                    AAri_cpp.Wire.transmit_stereo_to_stereo_mixer_2
                )
            case (4):
                entity = AAri_cpp.StereoMixer4.create(engine.engine)
                self.transmit_mono_func = AAri_cpp.Wire.transmit_mono_to_stereo_mixer_4
                self.transmit_stereo_func = (
                    AAri_cpp.Wire.transmit_stereo_to_stereo_mixer_4
                )
            case (8):
                entity = AAri_cpp.StereoMixer8.create(engine.engine)
                self.transmit_mono_func = AAri_cpp.Wire.transmit_mono_to_stereo_mixer_8
                self.transmit_stereo_func = (
                    AAri_cpp.Wire.transmit_stereo_to_stereo_mixer_8
                )
            case (16):
                entity = AAri_cpp.StereoMixer16.create(engine.engine)
                self.transmit_mono_func = AAri_cpp.Wire.transmit_mono_to_stereo_mixer_16
                self.transmit_stereo_func = (
                    AAri_cpp.Wire.transmit_stereo_to_stereo_mixer_16
                )
            case (32):
                entity = AAri_cpp.StereoMixer32.create(engine.engine)
                self.transmit_mono_func = AAri_cpp.Wire.transmit_mono_to_stereo_mixer_32
                self.transmit_stereo_func = (
                    AAri_cpp.Wire.transmit_stereo_to_stereo_mixer_32
                )
            case other:
                raise ValueError(
                    "Invalid size for StereoMixer, must be a power of 2 and smaller than 32"
                )
        super().__init__(entity, size)


class MonoMixer(MixerBlock):
    OUTPUTS = [ParamDef("out", AAri_cpp.Output1D)]

    def __init__(self, size: int = 4):
        from AAri.audio_engine import AudioEngine  # Avoid circular import

        engine = AudioEngine()
        match size:
            case (2):
                entity = AAri_cpp.MonoMixer2.create(engine.engine)
                self.transmit_func = AAri_cpp.Wire.transmit_to_mono_mixer_2
            case (4):
                entity = AAri_cpp.MonoMixer4.create(engine.engine)
                self.transmit_func = AAri_cpp.Wire.transmit_to_mono_mixer_4
            case (8):
                entity = AAri_cpp.MonoMixer8.create(engine.engine)
                self.transmit_func = AAri_cpp.Wire.transmit_to_mono_mixer_8
            case (16):
                entity = AAri_cpp.MonoMixer16.create(engine.engine)
                self.transmit_func = AAri_cpp.Wire.transmit_to_mono_mixer_16
            case (32):
                entity = AAri_cpp.MonoMixer32.create(engine.engine)
                self.transmit_func = AAri_cpp.Wire.transmit_to_mono_mixer_32
            case other:
                raise ValueError(
                    "Invalid size for StereoMixer, "
                    "must be a power of 2 and smaller than 32"
                )
        super().__init__(entity, size)


class ProductBase(Block):
    pass
