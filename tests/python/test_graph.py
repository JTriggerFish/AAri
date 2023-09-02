import unittest
import sys
from time import sleep

from AAri.audio_engine import AudioEngine
from AAri.oscillators import SineOsc


class TestAAriPythonBindings(unittest.TestCase):
    def test_sound(self):
        audio_engine = AudioEngine()
        audio_engine.startAudio()
        osc = SineOsc(freq=440.0, amplitude=0.8)
        audio_engine.out << osc
        sleep(3)
        audio_engine.stopAudio()
