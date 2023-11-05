from typing import List, Set, Dict, Any

import AAri_cpp  # Import the Pybind11 module
from AAri_cpp import Entity


class Block:
    def __init__(self, entity: Entity):
        self.entity = entity
        self.engine = AudioEngine()  # Get the unique instance
        self.cpp_block = self.engine.engine.view_block(entity)
        self.engine.blocks.add(self)

    def __del__(self):
        self.engine.blocks.remove(self)

    @property
    def input_ids(self) -> List[Entity]:
        return self.cpp_block.inputIds()

    @property
    def output_ids(self) -> List[Entity]:
        return self.cpp_block.outputIds()

    def view_inputs_outputs(self) -> Dict[Entity, Any]:
        return self.engine.engine.view_block_io(self.entity)


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
        # TODO add an output block
        self._blocks: Set[Block] = set()
        # self.engine.set_output_block(StereoMixerBase())

    @property
    def output_ref(self) -> (Entity, int):
        return self.engine.get_output_ref()

    def set_output_ref(self, entity: Entity, width: int):
        self.engine.set_output_ref(entity, width)

    def start(self):
        self.engine.startAudio()

    def stop(self):
        self.engine.stopAudio()

    def reset(self):
        self.stop()
        cls = type(self)
        cls._instance = None

    @property
    def _cpp_blocks(self) -> List[AAri_cpp.Block]:
        return self.engine.get_blocks()

    @property
    def blocks(self) -> Set[Block]:
        return self._blocks
