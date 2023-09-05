from collections import OrderedDict

import AAri_cpp  # Import the Pybind11 module

from AAri.audio_engine import Block


class SineOsc(Block):
    """Sine oscillator block"""

    INPUTS = OrderedDict(
        {
            "freq": AAri_cpp.SineOscInputs.FREQ,
            "amplitude": AAri_cpp.SineOscInputs.AMPLITUDE,
        }
    )
    OUTPUTS = OrderedDict({"out": AAri_cpp.SineOscOutputs.OUT})

    def __init__(self, freq: float, amplitude: float = 1.0):
        super().__init__(AAri_cpp.SineOsc(freq, amplitude))
