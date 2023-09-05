import sys
import typing
from collections import OrderedDict
import numpy as np

sys.path.append(
    "../../release/"
)  # Add the parent directory to the path so we can import the module

import AAri_cpp  # Import the Pybind11 module


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
    def value(self):
        if self.param.is_input:
            return self.block._graph.cpp_graph.get_block_inputs(
                self.block.block_ptr.id, self.param.idx
            )
        else:
            return self.block._graph.cpp_graph.get_block_outputs(
                self.block.block_ptr.id, self.param.idx
            )


class BlockWithParametersMeta(type):
    def __new__(cls, name, bases, dct):
        params = cls.create_params(dct.get("INPUTS", {}), False)
        params += cls.create_params(dct.get("OUTPUTS", {}), False)
        for param in params.values():
            # Define getter and setter
            def getter(self: "Block"):
                return AttachedParam(self, param)

            # Define setter
            def setter(
                self: "Block", value: typing.Union[float, np.ndarray, AttachedParam]
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
                        if value.param.width != param.width:
                            raise RuntimeError(
                                "Cannot connect block parameters with different input and output size"
                            )
                        self._graph.connect(
                            value.block, self, value.param.idx, param.width, param.idx
                        )
                    else:
                        # TODO IMPLEMENT THIS ON CPP SIDE
                        self._graph.cpp_graph.set_block_inputs(
                            self.block_ptr.id, param.width, value
                        )
                else:
                    raise RuntimeError("Cannot set output")

            # Create and set property
            prop = property(getter, setter)
            dct[param] = prop

        dct["_all_params"] = params

        return super(BlockWithParametersMeta, cls).__new__(cls, name, bases, dct)

    @staticmethod
    def create_params(
        params: typing.OrderedDict[str, int], is_input: bool
    ) -> typing.Dict[str, Param]:
        """
        Iterate through the parameters, check their indices are strictly increasing and
        set their width based on the difference with the next index

        :param params:
        :param is_input:
        :return:
        """
        ret = {}
        prev_idx = -1
        for param, idx in params.items():
            if idx <= prev_idx:
                raise RuntimeError(
                    f"Parameter indices must be strictly increasing, {param} has index {idx} <= {prev_idx}"
                )
            width = idx - prev_idx
            ret[param] = Param(param, idx, width, is_input)
            prev_idx = idx
        return ret


class Block(metaclass=BlockWithParametersMeta):
    def __init__(
        self,
        block_ptr: AAri_cpp.Block,
    ):
        """Base class for all blocks, wrapping cpp block"""
        self.block_ptr = block_ptr
        self._graph = AudioEngine().graph

    def __lshift__(self, block: "Block"):
        pass


class MixerBase(Block):
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


class AudioGraph:
    """Cpp graph wrapper"""

    def __init__(self, cpp_graph: AAri_cpp.AudioGraph):
        self.cpp_graph = cpp_graph
        self.output_block = None

    def add_block(self, block: Block):
        # First check if the block is already in the graph:
        if not self.cpp_graph.has_block(block.block_ptr.id):
            self.cpp_graph.add_block(block.block_ptr)

    def remove_block(self, block: Block):
        self.cpp_graph.remove_block(block.block_ptr)

    def connect(
        self,
        block_from: Block,
        block_to: Block,
        channel_from: int,
        width: int,
        channel_to: int,
    ) -> int:
        """
        Connects two blocks and returns wired id
        :param block_from:
        :param block_to:
        :param channel_from:
        :param width:
        :param channel_to:
        :return:
        """
        return self.cpp_graph.connect_wire(
            block_from.block_ptr.id,
            block_to.block_ptr.id,
            channel_from,
            width,
            channel_to,
        )

    def disconnect_wire(self, wire_id: int):
        self.cpp_graph.disconnect_wire(wire_id)


class AudioEngine:
    """
    Thread-safe Singleton class for the audio engine wrapper
    """

    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(AudioEngine, cls).__new__(cls)
            cls._instance._initialize()  # Call a separate initialization method
        return cls._instance

    def _initialize(self):
        """
        Any initialization logic should go here.
        """
        self.engine = AAri_cpp.AudioEngine()
        self._graph = AudioGraph(self.engine.get_graph())
        self.set_output_block(StereoMixerBase())

    def set_output_block(self, block: Block):
        self._graph.add_block(block)
        self.engine.set_output_block(block.block_ptr.id, 0)
        self.output_block = block

    @property
    def out(self) -> Block:
        return self.output_block

    def start(self):
        self.engine.startAudio()

    def stop(self):
        self.engine.stopAudio()

    @property
    def graph(self) -> AudioGraph:
        return self._graph
