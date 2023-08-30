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
    m.doc() = "AAri: A real-time audio engine"; // Module documentation

    py::class_<AudioEngine>(m, "AudioEngine", py::module_local())
            .def(py::init<>())
            .def("startAudio", &AudioEngine::startAudio)
            .def("stopAudio", &AudioEngine::stopAudio)
            .def("get_graph", &AudioEngine::getAudioGraph, py::return_value_policy::reference);

    py::class_<AudioGraph>(m, "AudioGraph", py::module_local())
            .def(py::init<>())
            .def("add_block", &AudioGraph::add_block, py::arg("block"), py::return_value_policy::reference);

    py::class_<Graph::Block>(m, "Block", py::module_local());
    py::class_<Mixer, Graph::Block>(m, "Mixer");
    py::class_<Oscillator, Graph::Block>(m, "Oscillator");

    py::class_<Affine, Mixer>(m, "Affine", py::module_local())
            .def(py::init<float, float>(),
                 py::arg("a") = 0.0f,
                 py::arg("b") = 1.0f);

    py::class_<MonoToStereo, Mixer>(m, "MonoToStereo", py::module_local())
            .def(py::init<float, float>(),
                 py::arg("amp_db") = -30.0f,
                 py::arg("panning") = 0.0f);

    py::class_<StereoMixer, Mixer>(m, "StereoMixer", py::module_local())
            .def(py::init<>());

    // SineOsc bindings
    py::enum_<SineOsc::Inputs>(m, "SineOscInputs")
            .value("FREQ", SineOsc::Inputs::FREQ)
            .value("AMP", SineOsc::Inputs::AMP);
    py::enum_<SineOsc::Outputs>(m, "SineOscOutputs")
            .value("OUT", SineOsc::Outputs::OUT);

    py::class_<SineOsc, Oscillator>(m, "SineOsc", py::module_local())
            .def(py::init<float, float>(),
                 py::arg("freq") = 110.0f,
                 py::arg("amplitude") = 1.0f);


}
