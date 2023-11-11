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
        return self.cpp_block.inputIds

    @property
    def output_ids(self) -> List[Entity]:
        return self.cpp_block.outputIds

    def view_inputs_outputs(self) -> Dict[Entity, Any]:
        return self.engine.engine.view_block_io(self.entity)


class StereoMixer(Block):
    def __init__(self, size: int = 4):
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
                raise ValueError("Invalid size for StereoMixer")
        super().__init__(entity)
        self.free_inputs = [1] * size


class AudioEngine:
    """
    Thread-safe Singleton class for the audio engine wrapper
    """

    _instance = None

    def __new__(cls):
        if cls._instance is None:
            cls._instance = super(AudioEngine, cls).__new__(cls)
            cls._instance._initialize()  # Call a separate initialization method
            cls._instance._set_default_output()
        return cls._instance

    def _initialize(self):
        """
        Any initialization logic should go here.
        """
        self.engine = AAri_cpp.AudioEngine()
        self._blocks: Set[Block] = set()

    def _set_default_output(self):
        self.output_mixer = StereoMixer(4)
        self.engine.set_output_ref(self.output_mixer.output_ids[0], 2)

    def _add_output_mixer(self, mixer_size: int = 4):
        mixer = AAri_cpp.StereoMixerBase.create(self.engine, mixer_size)

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
