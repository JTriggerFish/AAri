import AAri_cpp  # Import the Pybind11 module

from AAri.audio_engine import Block
from AAri.audio_engine import StereoMixerBase as StereoMixer


class MonoToStereo(Block):
    """Mono to stereo block"""

    def __init__(self, amp_db: float = -30.0, panning: float = 0.0):
        super().__init__(AAri_cpp.MonoToStereo(amp_db, panning))