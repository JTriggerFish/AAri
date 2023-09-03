import unittest
import sys
from time import sleep

from AAri.audio_engine import AudioEngine
from AAri.oscillators import SineOsc


class TestAAriPythonBindings(unittest.TestCase):
    def test_sound(self):
        audio_engine = AudioEngine()
        audio_engine.start()
        osc = SineOsc(freq=220.0, amplitude=1.0)
        audio_engine.out << osc
        osc2 = SineOsc(freq=440.0, amplitude=1.0)
        for freq in range(220, 440, 5):
            osc = SineOsc(freq=freq, amplitude=1.0)
            audio_engine.out << osc
        audio_engine.out << osc2
        sleep(3)
        audio_engine.stop()
