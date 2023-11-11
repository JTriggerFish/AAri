import sys
import unittest
from time import sleep

import AAri_cpp  # Import the Pybind11 module
from AAri.audio_engine import AudioEngine, Block
from AAri.oscillators import SineOsc

sys.path.append(r"../../AAri")


class TestEngine(unittest.TestCase):
    def test_engine(self):
        audio_engine = AudioEngine()
        self.assertIsNotNone(audio_engine)

        # Test starting and stopping the audio engine
        audio_engine.start()
        audio_engine.stop()

    def test_sine_osc_block_manual(self):
        audio_engine = AudioEngine()
        # TODO figure out why adding the block after starting the engine causes a segfault
        audio_engine.start()
        sine_osc_id = AAri_cpp.SineOsc.create(audio_engine.engine)
        sine_block = Block(sine_osc_id)
        audio_engine.engine.add_wire_to_mixer(
            sine_osc_id,
            audio_engine.output_mixer.entity,
            sine_block.output_ids[0],
            0,
            AAri_cpp.Wire.transmit_mono_to_stereo_mixer_4,
        )
        sleep(3)
        ios = []
        sine_outs = []
        for i in range(100):
            ios.append(sine_block.view_inputs_outputs())
        audio_engine.stop()


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
