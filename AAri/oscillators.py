from collections import OrderedDict

import AAri_cpp  # Import the Pybind11 module

from AAri.block import Block


class SineOsc(Block):
    """Sine oscillator block"""

    INPUT_SIZE = AAri_cpp.SineOsc.INPUT_SIZE
    OUTPUT_SIZE = AAri_cpp.SineOsc.OUTPUT_SIZE
    INPUTS = OrderedDict(
        {
            "freq": AAri_cpp.SineOsc.FREQ,
            "amp": AAri_cpp.SineOsc.AMP,
        }
    )
    OUTPUTS = OrderedDict({"out": AAri_cpp.SineOsc.OUT})

    def __init__(self, freq: float, amplitude: float = 1.0):
        super().__init__(AAri_cpp.SineOsc(freq, amplitude))
