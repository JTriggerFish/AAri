from __future__ import annotations

import sys
import typing
from collections import OrderedDict

sys.path.append(
    "../../release/"
)  # Add the parent directory to the path so we can import the module
import AAri_cpp  # Import the Pybind11 module
import numpy as np


def dB(db_val: float) -> float:
    """
    Convert a decibel value to a linear value
    :param db_val:
    :return:
    """
    return 10 ** (db_val / 20)


class Param:
    def __init__(self, name: str, idx: int, width: bool, is_input: bool):
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
    def value(self) -> typing.Union[float, np.ndarray]:
        if self.param.is_input:
            ret = self.block._graph.cpp_graph.get_block_inputs(
                self.block.block_ptr.id, self.param.idx, self.param.width
            )

        else:
            ret = self.block._graph.cpp_graph.get_block_outputs(
                self.block.block_ptr.id, self.param.idx, self.param.width
            )
        return ret[0] if self.param.width == 1 else ret

    def __add__(self, other: typing.Union[float, int, "AttachedParam", "ScaledParam"]):
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

    def __add__(self, other: typing.Union[float, int, AttachedParam, "ScaledParam"]):
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
        graph = self.attached_param.block._graph
        if not graph.cpp_graph.has_block(self.attached_param.block.block_ptr.id):
            graph.cpp_graph.add_block(self.attached_param.block.block_ptr)
        graph.connect(
            self.attached_param.block,
            input_param.block,
            self.attached_param.param.idx,
            self.attached_param.param.width,
            input_param.param.idx,
            self.gain,
            self.offset,
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
        mixer = MonoMixerBase() if width == 1 else StereoMixerBase()
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
        # Fail if INPUTS, OUTPUTS, INPUT_SIZE or OUTPUT_SIZE are not defined
        if "INPUT_SIZE" not in dct or "OUTPUT_SIZE" not in dct:
            raise RuntimeError(
                "INPUT_SIZE and OUTPUT_SIZE must be defined as static members of the class"
            )

        params = cls.create_params(
            dct.get("INPUTS", {}), True, dct.get("INPUT_SIZE", 0)
        )
        params |= cls.create_params(
            dct.get("OUTPUTS", {}), False, dct.get("OUTPUT_SIZE", 0)
        )
        for name, parameter in params.items():
            # Define getter and setter
            def getter(self: "Block", parameter=parameter):
                return AttachedParam(self, parameter)

            # Define setter
            def setter(
                self: "Block",
                value: typing.Union[float, np.ndarray, AttachedParam],
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
        params: typing.OrderedDict[str, int],
        is_input: bool,
        total_size: int,
    ) -> typing.Dict[str, Param]:
        """
        Iterate through the parameters, check their indices are strictly increasing and
        set their width based on the difference with the next index

        :param params:
        :param is_input:
        :param total_size:
        :return:
        """
        ret = {}
        # Iterate backward through the parameters. The width of the last params
        # is the difference between its start index and size of the inputs, and then the width of the parameter before
        # it is the difference between its start index and the start index of the next parameter etc
        last_idx = total_size
        for name, idx in reversed(params.items()):
            if last_idx is None:
                width = idx
            else:
                width = last_idx - idx
            ret[name] = Param(name, idx, width, is_input)
            last_idx = idx
        return ret


class Block(metaclass=BlockWithParametersMeta):
    INPUT_SIZE = 0
    OUTPUT_SIZE = 0

    def __init__(
        self,
        block_ptr: AAri_cpp.Block,
    ):
        from AAri.audio_engine import AudioEngine  # Avoid circular import

        """Base class for all blocks, wrapping cpp block"""
        self.block_ptr = block_ptr
        self._graph = AudioEngine().graph

    def __lshift__(self, block: "Block"):
        pass


class MixerBase(Block):
    INPUT_SIZE = 0
    OUTPUT_SIZE = 0

    def __init__(self, block_ptr):
        super().__init__(block_ptr)

    def _find_free_slot(self, width: int) -> int:
        """
        Go through all the wires plugged into this block and find a free slot wide enough to accomodate
        the input
        :param width:
        :return:
        """
        wires = self.block_ptr.wires
        free_inputs = np.zeros(self.block_ptr.input_size, dtype=int)
        for wire in wires:
            free_inputs[wire.out_index : wire.out_index + wire.width] = 1
        free_inputs = np.where(free_inputs == 0)[0]
        for i in range(len(free_inputs - width)):
            if np.array_equal(
                free_inputs[i : i + width],
                np.arange(free_inputs[i], free_inputs[i] + width),
            ):
                return free_inputs[i]
        raise RuntimeError("No free slot found")


class ProductBase(Block):
    INPUT_SIZE = AAri_cpp.Product.INPUT_SIZE
    OUTPUT_SIZE = AAri_cpp.Product.OUTPUT_SIZE
    OUTPUTS = OrderedDict(
        {"out1": AAri_cpp.Product.OUT1, "out2": AAri_cpp.Product.OUT2}
    )

    def __init__(self, product_type=AAri_cpp.ProductType.DUAL_CHANNELS):
        super().__init__(AAri_cpp.Product(product_type))


class MonoMixerBase(MixerBase):
    """Mono mixer block"""

    INPUT_SIZE = AAri_cpp.MonoMixer.INPUT_SIZE
    OUTPUT_SIZE = AAri_cpp.MonoMixer.OUTPUT_SIZE
    OUTPUTS = OrderedDict({"out": AAri_cpp.MonoMixer.OUT})

    def __init__(self):
        super().__init__(AAri_cpp.MonoMixer())


class StereoMixerBase(MixerBase):
    """Stereo mixer block"""

    INPUT_SIZE = AAri_cpp.StereoMixer.INPUT_SIZE
    OUTPUT_SIZE = AAri_cpp.StereoMixer.OUTPUT_SIZE

    OUTPUTS = OrderedDict({"left": 0, "right": 1})

    def __init__(self):
        super().__init__(AAri_cpp.StereoMixer())

    def __lshift__(self, block: Block):
        # Default to -30db
        gain = dB(-30.0)
        panning = 0.0
        self._graph.add_block(block)
        if block.block_ptr.output_size > 2:
            raise RuntimeError("Cannot connect block with more than 2 outputs")
        free_slot = self._find_free_slot(2)
        if block.block_ptr.output_size == 1:
            # Use a stereo wire:
            self._graph.connect(
                block,
                self,
                0,
                2,
                free_slot,
                gain,
                0.0,
                AAri_cpp.WireTransform.STEREO_PAN,
                panning,
            )
        else:
            self._graph.connect(block, self, 0, 2, free_slot)
