import AAri_cpp  # Import the Pybind11 module
from AAri.audio_engine import AudioEngine
from AAri.block import Block, ParamDef


class SineOsc(Block):
    """Sine oscillator block"""

    INPUTS = [
        ParamDef("phase", AAri_cpp.Input1D),
        ParamDef("freq", AAri_cpp.Input1D),
        ParamDef("amp", AAri_cpp.Input1D),
    ]
    OUTPUTS = [
        ParamDef("out", AAri_cpp.Output1D),
    ]

    def __init__(self, freq: float, amplitude: float = 1.0):
        audio_engine = AudioEngine()
        sine_osc_id = AAri_cpp.SineOsc.create(audio_engine.engine)
        super().__init__(sine_osc_id)
