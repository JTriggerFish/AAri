import unittest
import sys
from time import sleep

from AAri.audio_engine import AudioEngine
from AAri.oscillators import SineOsc


class TestBasicGraphs(unittest.TestCase):
    def test_mini_saw(self):
        audio_engine = AudioEngine()
        audio_engine.start()
        base_freq = 220
        for mul in range(1, 32):
            osc = SineOsc(freq=mul*base_freq, amplitude=1.0 / mul)
            audio_engine.out << osc
        sleep(3)
        audio_engine.stop()
