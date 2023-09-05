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
            .def("get_graph", &AudioEngine::getAudioGraph)
            .def("get_audio_device", &AudioEngine::get_audio_device)
            .def("set_output_block", &AudioEngine::set_output_block, py::arg("node_index"),
                 py::arg("block_output_index"));

    py::class_<AudioGraph>(m, "AudioGraph", py::module_local())
            .def(py::init<uint32_t>())
            .def("add_block", &AudioGraph::add_block, py::arg("block"))
            .def("remove_block", &AudioGraph::remove_block, py::arg("block_id"))
            .def("connect_wire", &AudioGraph::connect_wire, py::arg("in_block_id"), py::arg("out_block_id"),
                 py::arg("in_index"), py::arg("width"), py::arg("out_index"))
            .def("disconnect_wire", &AudioGraph::disconnect_wire, py::arg("wire_id"),
                 py::arg("out_block_id") = py::none())
            .def("has_block", &AudioGraph::has_block, py::arg("block_id"))
            .def("get_all_blocks", &AudioGraph::py_get_all_blocks)
            .def("get_topological_order", &AudioGraph::py_get_topological_order)
            .def("get_block_inputs", &AudioGraph::py_get_block_inputs, py::arg("block_id"))
            .def("get_block_outputs", &AudioGraph::py_get_block_outputs, py::arg("block_id"));

    py::class_<Graph::Block, std::shared_ptr<Graph::Block>>(m, "Block", py::module_local())
            .def_property_readonly("id", &Graph::Block::id)
            .def_property_readonly("name", &Graph::Block::name)
            .def_property_readonly("input_size", &Graph::Block::input_size)
            .def_property_readonly("output_size", &Graph::Block::output_size)
            .def_readonly("last_processed_time", &Graph::Block::last_processed_time)
            .def_property_readonly("wires", &Graph::Block::py_get_input_wires);

    py::class_<Graph::Wire>(m, "Wire", py::module_local())
            .def_readonly("input", &Graph::Wire::in)
            .def_readonly("output", &Graph::Wire::out)
            .def_readonly("in_index", &Graph::Wire::in_index)
            .def_readonly("width", &Graph::Wire::width)
            .def_readonly("out_index", &Graph::Wire::out_index);

    py::class_<Mixer, Graph::Block, std::shared_ptr<Mixer>>(m, "Mixer");
    py::class_<Oscillator, Graph::Block, std::shared_ptr<Oscillator>>(m, "Oscillator");

    py::class_<Affine, Mixer, std::shared_ptr<Affine>>(m, "Affine", py::module_local())
            .def(py::init<float, float>(),
                 py::arg("a") = 0.0f,
                 py::arg("b") = 1.0f);

    py::class_<MonoToStereo, Mixer, std::shared_ptr<MonoToStereo>>(m, "MonoToStereo", py::module_local())
            .def(py::init<float, float>(),
                 py::arg("amp_db") = -30.0f,
                 py::arg("panning") = 0.0f);

    py::class_<StereoMixer, Mixer, std::shared_ptr<StereoMixer>>(m, "StereoMixer", py::module_local())
            .def(py::init<>());

    // SineOsc bindings
    py::enum_<SineOsc::Inputs>(m, "SineOscInputs")
            .value("FREQ", SineOsc::Inputs::FREQ)
            .value("AMP", SineOsc::Inputs::AMP);
    py::enum_<SineOsc::Outputs>(m, "SineOscOutputs")
            .value("OUT", SineOsc::Outputs::OUT);

    py::class_<SineOsc, Oscillator, std::shared_ptr<SineOsc>>(m, "SineOsc", py::module_local())
            .def(py::init<float, float>(),
                 py::arg("freq") = 110.0f,
                 py::arg("amplitude") = 1.0f);


}
