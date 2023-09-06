import sys
import typing
from collections import OrderedDict

sys.path.append(
    "../../release/"
)  # Add the parent directory to the path so we can import the module
import AAri_cpp  # Import the Pybind11 module
import numpy as np


class Param:
    def __init__(self, name: str, idx: int, width: bool, is_input: bool):
        self.name = name
        self.idx = idx
        self.width = width
        self.is_input = is_input

class AttachedParam:
    def __init__(self, block: "Block", param: Param):
        self.block = block
        self.param = param

    @property
    def value(self) -> typing.Union[float, np.ndarray]:
        if self.param.is_input:
            ret = self.block._graph.cpp_graph.get_block_inputs(
                self.block.block_ptr.id, self.param.idx, self.param.width)

        else:
            ret = self.block._graph.cpp_graph.get_block_outputs(
                self.block.block_ptr.id, self.param.idx, self.param.width
            )
        return ret[0] if self.param.width == 1 else ret


class BlockWithParametersMeta(type):
    def __new__(cls, name, bases, dct):
        # Fail if INPUTS, OUTPUTS, INPUT_SIZE or OUTPUT_SIZE are not defined
        if "INPUT_SIZE" not in dct or "OUTPUT_SIZE" not in dct:
            raise RuntimeError("INPUT_SIZE and OUTPUT_SIZE must be defined as static members of the class")

        params = cls.create_params(dct.get("INPUTS", {}), True, dct.get("INPUT_SIZE", 0))
        params |= cls.create_params(dct.get("OUTPUTS", {}), False, dct.get("OUTPUT_SIZE", 0))
        for name, parameter in params.items():
            # Define getter and setter
            def getter(self: "Block", parameter=parameter):
                return AttachedParam(self, parameter)

            # Define setter
            def setter(
                self: "Block", value: typing.Union[float, np.ndarray, AttachedParam], parameter=parameter
            ):
                """
                Set the value of a parameter to either a numerical
                value or the output parameter of a block
                :param self:
                :param value:
                :return:
                """
                if param.is_input:
                    if isinstance(value, AttachedParam):
                        if value.param.is_input:
                            raise RuntimeError("Cannot connect inputs to inputs")
                        if value.param.width != parameter.width:
                            raise RuntimeError(
                                "Cannot connect block parameters with different input and output size"
                            )
                        self._graph.connect(
                            value.block, self, value.param.idx, parameter.width, parameter.idx
                        )
                    else:
                        # TODO IMPLEMENT THIS ON CPP SIDE
                        self._graph.cpp_graph.set_block_inputs(
                            self.block_ptr.id, parameter.width, value
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
        params: typing.OrderedDict[str, int], is_input: bool,
        total_size : int,
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
        from AAri.audio_engine import AudioEngine # Avoid circular import
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


class StereoMixerBase(MixerBase):
    """Stereo mixer block"""

    INPUT_SIZE = AAri_cpp.StereoMixer.INPUT_SIZE
    OUTPUT_SIZE = AAri_cpp.StereoMixer.OUTPUT_SIZE

    OUTPUTS = OrderedDict({"left": 0, "right": 1})

    def __init__(self):
        super().__init__(AAri_cpp.StereoMixer())

    def __lshift__(self, block: Block):
        self._graph.add_block(block)
        if block.block_ptr.output_size > 2:
            raise RuntimeError("Cannot connect block with more than 2 outputs")
        if block.block_ptr.output_size == 1:
            # TODO just replace by an expanding wire
            stereo = Block(AAri_cpp.MonoToStereo(amp_db=-30.0, panning=0.5))
            self._graph.add_block(stereo)
            self._graph.connect(block, stereo, 0, 1, 0)
            block = stereo
        free_slot = self._find_free_slot(2)
        self._graph.connect(block, self, 0, 2, free_slot)

