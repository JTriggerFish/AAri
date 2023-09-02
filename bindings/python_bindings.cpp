#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <../../src/core/graph.h>
#include <../../src/core/graph_io.h>
#include <../../src/core/audio_engine.h>
#include <../../src/blocks/oscillators.h>
#include <../../src/blocks/mixers.h>

namespace py = pybind11;
using namespace Graph;

PYBIND11_MODULE(AAri_cpp, m) {
    m.doc() = "AAri_cpp: Real-time audio engine backend"; // Module documentation

    py::class_<AudioEngine>(m, "AudioEngine", py::module_local())
            .def(py::init<>())
            .def("startAudio", &AudioEngine::startAudio)
            .def("stopAudio", &AudioEngine::stopAudio)
            .def("get_graph", &AudioEngine::getAudioGraph, py::return_value_policy::reference)
            .def("set_output_node", &AudioEngine::set_output_node, py::arg("node_index"), py::arg("block_output_index"));

    py::class_<AudioGraph>(m, "AudioGraph", py::module_local())
            .def(py::init<>())
            .def("add_block", &AudioGraph::add_block, py::arg("block"), py::return_value_policy::reference)
            .def("remove_block", &AudioGraph::remove_block, py::arg("block_id"))
            .def("connect_wire", &AudioGraph::connect_wire, py::arg("in_block_id"), py::arg("out_block_id"),
                 py::arg("in_index"), py::arg("width"), py::arg("out_index"))
            .def("disconnect_wire", &AudioGraph::disconnect_wire, py::arg("wire_id"), py::arg("out_block_id") = py::none());

    py::class_<Graph::Block>(m, "Block", py::module_local())
            .def("id", &Graph::Block::id)
            .def("input_size", &Graph::Block::input_size)
            .def("output_size", &Graph::Block::output_size)
            .def("wires", &Graph::Block::py_get_input_wires);

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
