import AAri_cpp  # Import the Pybind11 module

from AAri.audio_engine import Block


class SineOsc(Block):
    """Sine oscillator block"""

    def __init__(self, freq: float, amplitude: float = 1.0):
        super().__init__(AAri_cpp.SineOsc(freq, amplitude))
