import sys
import unittest
from time import sleep

from AAri.audio_engine import AudioEngine
from AAri.oscillators import SineOsc

sys.path.append(r"../../AAri")


class TestBasicGraphs(unittest.TestCase):
    def test_mini_saw_and_input_get(self):
        audio_engine = AudioEngine()
        audio_engine.start()
        base_freq = 220
        for mul in range(1, 32):
            osc = SineOsc(freq=mul * base_freq, amplitude=1.0 / mul)
            audio_engine.out << osc
            assert osc.freq.value == mul * base_freq
        sleep(3)
        audio_engine.reset()

    def test_set_input(self):
        audio_engine = AudioEngine()
        audio_engine.start()
        base_freq = 110
        osc = SineOsc(freq=base_freq, amplitude=1.0)
        audio_engine.out << osc
        for mul in range(1, 32):
            osc.freq = mul * base_freq
            assert osc.freq.value == mul * base_freq
            sleep(0.1)
        audio_engine.reset()

    def test_set_param_fm(self):
        audio_engine = AudioEngine()
        audio_engine.start()
        base_freq = 110
        osc1 = SineOsc(freq=base_freq, amplitude=1.0)
        osc2 = SineOsc(freq=40, amplitude=1.0)
        audio_engine.out << osc1
        osc1.freq = 220 + 110 * osc2.out
        sleep(3)
        audio_engine.reset()
