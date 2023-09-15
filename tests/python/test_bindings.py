import unittest
import sys

sys.path.append(r"../../AAri")

import AAri_cpp


class TestAAriPythonBindings(unittest.TestCase):
    def test_audio_engine(self):
        audio_engine = AAri_cpp.AudioEngine()
        self.assertIsNotNone(audio_engine)

        # Test starting and stopping the audio engine
        audio_engine.startAudio()
        audio_engine.stopAudio()

    def test_sine_osc_block(self):
        sine_osc = AAri_cpp.SineOsc(freq=440.0, amplitude=0.8)
        self.assertIsNotNone(sine_osc)


if __name__ == "__main__":
    unittest.main()
