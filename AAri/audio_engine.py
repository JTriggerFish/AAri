from time import sleep
from typing import List, Set

import AAri_cpp  # Import the Pybind11 module
from AAri_cpp import Entity
from block import Block, AttachedParam, MixerBlock, StereoMixer


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

    @property
    def output_ref(self) -> (Entity, int):
        return self.engine.get_output_ref()

    def set_output_ref(self, entity: Entity, width: int):
        self.engine.set_output_ref(entity, width)

    def start(self):
        self.engine.startAudio()
        # To avoid race conditions with other pybind11 functions
        # calling into the engine while the audio thread is starting
        sleep(0.0001)

    def stop(self):
        self.engine.stopAudio()

    def reset(self):
        self.stop()
        cls = type(self)
        cls._instance = None

    def get_wires_to_block(self, block: Block) -> List[AAri_cpp.Wire]:
        wires_ids = self.engine.get_wires_to_block(block.entity)
        wires = [self.engine.view_wire(w) for w in wires_ids]
        return wires

    def add_wire(
        self,
        source: AttachedParam,
        target: AttachedParam | MixerBlock,
        gain: float = 1.0,
        offset: float = 0.0,
    ):
        assert not source.param.is_input
        assert source.param.is_input
        target_block = target if isinstance(target, Block) else target.block

        if isinstance(target_block, MixerBlock):
            self._add_wire_to_mixer(source, target)

        match source.param.width, target.param.width:
            case (1, 1):
                self.engine.add_wire(
                    source.block.entity,
                    target.block.entity,
                    source.param.idx,
                    target.param.idx,
                    AAri_cpp.Wire.transmit_1d_to_1d,
                    gain=gain,
                    offset=offset,
                )
            case (1, 2):
                self.engine.add_wire(
                    source.block.entity,
                    target.block.entity,
                    source.param.idx,
                    target.param.idx,
                    AAri_cpp.Wire.broadcast_1d_to_2d,
                    gain=gain,
                    offset=offset,
                )
            case other:
                raise ValueError(
                    f"Invalid wire width combination: {source.param.width}, {target.param.width}"
                )

    def _add_wire_to_mixer(self, source: AttachedParam, target: MixerBlock):
        """Match / case on the mixer size and whether the mixer is stereo
        or mono and the input is 1d or 2d / stereo"""
        free_input = target.find_free_slot()
        match source.param.width, type(target):
            case (1, MonoMixer):
                self.engine.add_wire_to_mixer(
                    source.block.entity,
                    target.entity,
                    source.param.idx,
                    free_input,
                    target.transmit_func,
                )
            case (1, StereoMixer):
                self.engine.add_wire_to_mixer(
                    source.block.entity,
                    target.entity,
                    source.param.idx,
                    free_input,
                    target.transmit_mono_func,
                )
            case (2, StereoMixer):
                self.engine.add_wire_to_mixer(
                    source.block.entity,
                    target.entity,
                    source.param.idx,
                    free_input,
                    target.transmit_stereo_func,
                )
            case other:
                raise ValueError(
                    f"Invalid wire width combination: {source.param.width}, {target.param.width}"
                )

    @property
    def _cpp_blocks(self) -> List[AAri_cpp.Block]:
        return self.engine.get_blocks()

    @property
    def blocks(self) -> Set[Block]:
        return self._blocks
