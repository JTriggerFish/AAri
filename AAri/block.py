from __future__ import annotations

from collections import OrderedDict
from typing import List, Dict, Any, Union

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


class Param:
    def __init__(self, name: str, idx: Entity, width: int, is_input: bool):
        self.name = name
        self.idx = idx
        self.width = width
        self.is_input = is_input


class ParamExpression:
    def connect(self, input_param: "AttachedParam"):
        pass


class AttachedParam(ParamExpression):
    def __init__(self, block: "Block", param: Param):
        self.block = block
        self.param = param

    @property
    def value(self) -> Union[float, np.ndarray]:
        return self.block.view_inputs_outputs()[self.param.idx]

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
        params_list = []
        if isinstance(param1, ScaledParam):
            params_list.append(param1)
        else:
            params_list += param1.params_list
        if isinstance(param2, ScaledParam):
            params_list.append(param2)
        else:
            params_list += param2.params_list
        self.params_list = params_list

    def connect(self, input_param: AttachedParam):
        """
        First create a mono or stereo mixer after checking all the blocks outputs are of width 1 or 2,
        and then connect that to the input_param
        :param input_param:
        :return:
        """
        # Check all the blocks are of the same width =  1 or 2
        width = self.params_list[0].attached_param.param.width
        for param in self.params_list:
            if param.attached_param.param.width != width:
                raise RuntimeError(
                    "Cannot connect parameters with different widths to a single input"
                )
        if width > 2:
            raise RuntimeError(
                "Cannot connect more than 2 parameters to a single input"
            )
        if width != input_param.param.width:
            raise RuntimeError(
                "Cannot connect block parameters with different input and output size"
            )
        mixer = MonoMixer() if width == 1 else StereoMixer()
        if len(self.params_list) > mixer.INPUT_SIZE / width:
            raise RuntimeError(
                f"Cannot connect more than {mixer.INPUT_SIZE / width} parameters to a single input"
            )
        for i, param in enumerate(self.params_list):
            input_param._graph.connect(
                param.attached_param.block,
                mixer,
                param.attached_param.param.idx,
                param.attached_param.param.width,
                i * param.attached_param.param.width,
                param.gain,
                param.offset,
            )
        mixer._graph.connect(
            mixer,
            input_param.block,
            0,
            mixer.OUTPUT_SIZE,
            input_param.param.idx,
        )


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
            def getter(self: "Block", parameter=parameter):
                return AttachedParam(self, parameter)

            # Define setter
            def setter(
                self: "Block",
                value: Union[float, np.ndarray, AttachedParam],
                parameter=parameter,
            ):
                """
                Set the value of a parameter to either a numerical
                value or the output parameter of a block
                :param self:
                :param value:
                :return:
                """
                if parameter.is_input:
                    if isinstance(value, ParamExpression):
                        value.connect(AttachedParam(self, parameter))
                    else:
                        value = np.array(value)
                        self._graph.cpp_graph.set_block_inputs(
                            self.block_ptr.id, parameter.idx, value
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
        params: OrderedDict[str, int],
        is_input: bool,
    ) -> Dict[str, Param]:
        """
        Iterate through the parameters, check their indices are strictly increasing and
        set their width based on the difference with the next index

        :param params:
        :param is_input:
        :param total_size:
        :return:
        """
        ret = {}
        # TODO - redo this with entt backend
        return ret


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

    def _find_free_slot(self) -> int:
        wires = self.engine.get_wires_to_block(self)
        used_inputs = np.zeros(self.num_inputs)
        # Find the free inputs
        for wire in wires:
            used_inputs[wire.to_input] = 1
        free_slots = np.where(used_inputs == 0)
        if len(free_slots) == 0:
            raise RuntimeError("No free inputs")
        return free_slots[0]


class StereoMixer(MixerBlock):
    def __init__(self, size: int = 4):
        from AAri.audio_engine import AudioEngine  # Avoid circular import

        engine = AudioEngine()
        match size:
            case (2):
                entity = AAri_cpp.StereoMixer2.create(engine.engine)
            case (4):
                entity = AAri_cpp.StereoMixer4.create(engine.engine)
            case (8):
                entity = AAri_cpp.StereoMixer8.create(engine.engine)
            case (16):
                entity = AAri_cpp.StereoMixer16.create(engine.engine)
            case (32):
                entity = AAri_cpp.StereoMixer32.create(engine.engine)
            case other:
                raise ValueError(
                    "Invalid size for StereoMixer, must be a power of 2 and smaller than 32"
                )
        super().__init__(entity, size)
        self.free_inputs = [1] * size


class MonoMixer(MixerBlock):
    def __init__(self, size: int = 4):
        from AAri.audio_engine import AudioEngine  # Avoid circular import

        engine = AudioEngine()
        match size:
            case (2):
                entity = AAri_cpp.MonoMixer2.create(engine.engine)
            case (4):
                entity = AAri_cpp.MonoMixer4.create(engine.engine)
            case (8):
                entity = AAri_cpp.MonoMixer8.create(engine.engine)
            case (16):
                entity = AAri_cpp.MonoMixer16.create(engine.engine)
            case (32):
                entity = AAri_cpp.MonoMixer32.create(engine.engine)
            case other:
                raise ValueError(
                    "Invalid size for StereoMixer, "
                    "must be a power of 2 and smaller than 32"
                )
        super().__init__(entity, size)
        self.free_inputs = [1] * size


class ProductBase(Block):
    pass
