import AAri_cpp  # Import the Pybind11 module

from AAri.block import Block, StereoMixerBase


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
        gain: float = 1.0,
        offset: float = 0.0,
        wire_transform: AAri_cpp.WireTransform = AAri_cpp.WireTransform.NONE,
        wire_transform_param: float = 0.0,
    ) -> int:
        """
        Connects two blocks and returns wired id
        :param block_from:
        :param block_to:
        :param channel_from:
        :param width:
        :param channel_to:
        :param gain:
        :param offset:
        :param wire_transform:
        :param wire_transform_param:
        :return:
        """
        return self.cpp_graph.connect_wire(
            block_from.block_ptr.id,
            block_to.block_ptr.id,
            channel_from,
            width,
            channel_to,
            gain,
            offset,
            wire_transform,
            wire_transform_param,
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

    def reset(self):
        self.stop()
        cls = type(self)
        cls._instance = None

    @property
    def graph(self) -> AudioGraph:
        return self._graph
