
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <../../src/core/graph.h>
#include <../../src/core/graph_io.h>
#include <../../src/core/audio_engine.h>
#include <../../src/blocks/oscillators.h>
#include <../../src/blocks/mixers.h>


namespace py = pybind11;
using namespace Graph;

PYBIND11_MODULE(AAri, m) {
    m.doc() = "AAri audio engine Python bindings";

    // AudioEngine bindings
    py::class_<AudioEngine>(m, "AudioEngine")
            .def(py::init<>())
            .def("startAudio", &AudioEngine::startAudio)
            .def("stopAudio", &AudioEngine::stopAudio);

    py::class_<Graph::AudioGraph>(m, "AudioGraph")
            .def(py::init<>())
            .def("add_block", &AudioGraph::add_block)
            .def("remove_block", &AudioGraph::remove_block)
            .def("connect_wire", &AudioGraph::connect_wire)
            .def("disconnect_wire", &AudioGraph::disconnect_wire);

    // MonoToStereo bindings
    py::class_<Graph::Block>(m, "Block");
    py::class_<Mixer, Graph::Block>(m, "Mixer");
    py::class_<Oscillator, Graph::Block>(m, "Oscillator");

    py::class_<Affine, Mixer>(m, "Affine")
            .def(py::init<float, float>(),
                 py::arg("a") = 0.0f,
                 py::arg("b") = 1.0f);

    py::class_<MonoToStereo, Mixer>(m, "MonoToStereo")
            .def(py::init<float, float>(),
                 py::arg("amp_db") = -30.0f,
                 py::arg("panning") = 0.0f);

    py::class_<StereoMixer, Mixer>(m, "StereoMixer")
            .def(py::init<>());

    // SineOsc bindings
    py::class_<SineOsc, Oscillator>(m, "SineOsc")
            .def(py::init<float, float>(),
                 py::arg("freq") = 110.0f,
                 py::arg("amplitude") = 1.0f);
}
