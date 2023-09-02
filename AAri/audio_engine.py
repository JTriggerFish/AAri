import sys
sys.path.append(
    "../../release/"
)  # Add the parent directory to the path so we can import the module

import AAri_cpp  # Import the Pybind11 module
import numpy as np


class Block:
    """Base class for all blocks, wrapping cpp block"""

    def __init__(self, block_ptr):
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
        wires = self.block_ptr.wires()
        free_inputs = np.zeros(self.block_ptr.input_size(), dtype=int)
        for wire in wires:
            free_inputs[wire.in_index : wire.in_index + wire.width] = 1
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

    def __init__(self):
        super().__init__(AAri_cpp.StereoMixer())

    def __lshift__(self, block: Block):
        self._graph.add_block(block)
        free_slot = self._find_free_slot(2)
        if block.block_ptr.output_size() > 2:
            raise RuntimeError("Cannot connect block with more than 2 outputs")
        if block.block_ptr.output_size() == 1:
                stereo = Block(AAri_cpp.MonoToStereo(amp_db=-30.0, panning=0.5))
                self._graph.add_block(stereo)
                self._graph.connect(block, stereo, 0, 1, free_slot)
                block = stereo
        self._graph.connect(block, self, 0, 2, free_slot)


class AudioGraph:
    """Cpp graph wrapper"""

    def __init__(self, cpp_graph: AAri_cpp.AudioGraph):
        self.graph = cpp_graph
        self.output_block = None

    def add_block(self, block: Block):
        # First check if the block is already in the graph:
        if not self.graph.has_block(block.block_ptr.id()):
            self.graph.add_block(block.block_ptr)

    def remove_block(self, block: Block):
        self.graph.remove_block(block.block_ptr)

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
        return self.graph.connect_wire(
            block_from.block_ptr.id(),
            block_to.block_ptr.id(),
            channel_from,
            width,
            channel_to,
        )

    def disconnect_wire(self, wire_id: int):
        self.graph.disconnect_wire(wire_id)


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
        self.engine.set_output_block(block.block_ptr.id(), 0)
        self.output_block = block

    @property
    def out(self) -> Block:
        return self.output_block

    def start(self):
        self.engine.startAudio()

    def stop(self):
        self.engine.stopAudio()

    @property
    def graph(self):
        return self._graph
