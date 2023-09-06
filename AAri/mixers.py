from collections import OrderedDict

import AAri_cpp  # Import the Pybind11 module

from AAri.block import Block


class MonoToStereo(Block):
    """Mono to stereo block"""

    INPUT_SIZE = AAri_cpp.MonoToStereo.INPUT_SIZE
    OUTPUT_SIZE = AAri_cpp.MonoToStereo.OUTPUT_SIZE
    OUTPUTS = OrderedDict({"left": 0, "right": 1})

    def __init__(self, amp_db: float = -30.0, panning: float = 0.0):
        super().__init__(AAri_cpp.MonoToStereo(amp_db, panning))
