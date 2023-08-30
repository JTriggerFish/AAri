import unittest
import sys

sys.path.append('../../debug/')  # Add the parent directory to the path so we can import the module

import AAri  # Assuming the compiled module is named 'AAri'


class TestAAriPythonBindings(unittest.TestCase):

    def test_audio_engine(self):
        audio_engine = AAri.AudioEngine()
        self.assertIsNotNone(audio_engine)

        # Test starting and stopping the audio engine
        audio_engine.startAudio()
        audio_engine.stopAudio()

    def test_mono_to_stereo_block(self):
        mono_to_stereo = AAri.MonoToStereo(amp_db=-20.0, panning=0.5)
        self.assertIsNotNone(mono_to_stereo)

    def test_sine_osc_block(self):
        sine_osc = AAri.SineOsc(freq=440.0, amplitude=0.8)
        self.assertIsNotNone(sine_osc)


if __name__ == '__main__':
    unittest.main()
