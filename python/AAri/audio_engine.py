import AAri_cpp  # Import the Pybind11 module
import time
import re


class Block:
    """Base class for all blocks, wrapping cpp block"""

    def __init__(self, block_ptr):
        self.block_ptr = block_ptr
        self._graph = AudioEngine().graph


class StereoMixer(Block):
    """Stereo mixer block"""

    def __init__(self):
        super().__init__(AAri_cpp.StereoMixer())


class AudioGraph:
    """Cpp graph wrapper"""

    def __init__(self):
        self.graph = AAri_cpp.AudioGraph()
        self.output_block = StereoMixer()
        self.graph.add_block(self.output_block.block_ptr)

    def add_block(self, block: Block):
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
            block_from.block_ptr.id,
            block_to.block_ptr.id,
            channel_from,
            width,
            channel_to,
        )

    def disconnect_wire(self, wire_id: int):
        self.graph.disconnect_wire(wire_id)

    def __lshift__(self, block: Block):
        self.add_block(block)


class AudioEngine:
    """
    Singleton class for the audio engine wrapper
    """

    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(AudioEngine, cls).__new__(cls)
            cls._instance.engine = AAri_cpp.AudioEngine()
            cls._instance._graph = AudioGraph()
            cls._instance.engine.set_output_block(
                cls._instance._graph.output_block.block_ptr.id
            )
        return cls._instance

    def start(self):
        self.engine.startAudio()

    def stop(self):
        self.engine.stopAudio()

    @property
    def graph(self):
        return self._graph
