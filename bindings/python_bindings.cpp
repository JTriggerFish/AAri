
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <../../src/core/graph.h>
#include <../../src/core/graph_io.h>
#include <../../src/core/audio_engine.h>
#include <../../src/blocks/oscillators.h>
#include <../../src/blocks/mixers.h>


namespace py = pybind11;

PYBIND11_MODULE(AAri, m) {
    m.doc() = "AAri audio engine Python bindings";

    // AudioEngine bindings
    py::class_<AudioEngine>(m, "AudioEngine")
            .def(py::init<>())
            .def("startAudio", &AudioEngine::startAudio)
            .def("stopAudio", &AudioEngine::stopAudio);

    // MonoToStereo bindings
    py::class_<MonoToStereo, Mixer>(m, "MonoToStereo")
            .def(py::init<float, float>(),
                 py::arg("amp_db") = -30.0f,
                 py::arg("panning") = 0.0f);

    // SineOsc bindings
    py::class_<SineOsc, Oscillator>(m, "SineOsc")
            .def(py::init<float, float>(),
                 py::arg("freq") = 110.0f,
                 py::arg("amplitude") = 1.0f);
}
