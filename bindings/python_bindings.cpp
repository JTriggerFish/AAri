#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../src/core/graph.h"
#include "../src/core/wire.h"
#include "../src/core/graph_io.h"
#include "../src/core/audio_engine.h"
#include "../src/blocks/oscillators.h"
#include "../src/blocks/mixers.h"
#include "../src/blocks/envelopes.h"

namespace py = pybind11;
using namespace Graph;

PYBIND11_MODULE(AAri_cpp, m) {
    m.doc() = "AAri_cpp: Real-time audio engine backend"; // Module documentation

    py::enum_<WireTransform>(m, "WireTransform")
            .value("NONE", WireTransform::NONE)
            .value("EXPAND", WireTransform::EXPAND)
            .value("STEREO_PAN", WireTransform::STEREO_PAN)
            .export_values();

    py::class_<Wire>(m, "Wire", py::module_local())
            .def_readonly("input", &Wire::in)
            .def_readonly("output", &Wire::out)
            .def_readonly("in_index", &Wire::in_index)
            .def_readonly("width", &Wire::width)
            .def_readonly("out_index", &Wire::out_index)
            .def_readonly("gain", &Wire::gain)
            .def_readonly("offset", &Wire::offset)
            .def_readonly("transform", &Wire::transform)
            .def_readonly("wire_transform_param", &Wire::wire_transform_param);

    py::class_<Graph::Block, std::shared_ptr<Graph::Block>>(m, "Block", py::module_local())
            .def_property_readonly("id", &Graph::Block::id)
            .def_property_readonly("name", &Graph::Block::name)
            .def_property_readonly("input_size", &Graph::Block::input_size)
            .def_property_readonly("output_size", &Graph::Block::output_size)
            .def_readonly("last_processed_time", &Graph::Block::last_processed_time)
            .def_property_readonly("wires", &Graph::Block::py_get_input_wires);


    py::class_<AudioGraph>(m, "AudioGraph", py::module_local())
            .def(py::init([](std::shared_ptr<AudioEngine> engine) {
                return new AudioGraph(engine.get());
            }))
            .def("add_block", &AudioGraph::add_block, py::arg("block"))
            .def("remove_block", &AudioGraph::remove_block, py::arg("block_id"))
            .def("connect_wire", &AudioGraph::connect_wire, py::arg("in_block_id"), py::arg("out_block_id"),
                 py::arg("in_index"), py::arg("width"), py::arg("out_index"),
                 py::arg("gain") = 1.0f,
                 py::arg("offset") = 0.0f,
                 py::arg("transform") = WireTransform::NONE,
                 py::arg("wire_transform_param") = 0.0f)
            .def("disconnect_wire", &AudioGraph::disconnect_wire, py::arg("wire_id"),
                 py::arg("out_block_id") = py::none())
            .def("has_block", &AudioGraph::has_block, py::arg("block_id"))
            .def("get_all_blocks", &AudioGraph::py_get_all_blocks)
            .def("get_topological_order", &AudioGraph::py_get_topological_order)
            .def("get_block_inputs", &AudioGraph::py_get_block_inputs, py::arg("block_id"), py::arg("input_index"),
                 py::arg("width"))
            .def("set_block_inputs", &AudioGraph::py_set_block_inputs, py::arg("block_id"), py::arg("input_index"),
                 py::arg("input"))
            .def("get_block_outputs", &AudioGraph::py_get_block_outputs, py::arg("block_id"),
                 py::arg("output_index"),
                 py::arg("width"))
            .def("tweak_wire_gain", &AudioGraph::tweak_wire_gain, py::arg("wire_id"), py::arg("gain"))
            .def("tweak_wire_offset", &AudioGraph::tweak_wire_offset, py::arg("wire_id"), py::arg("offset"))
            .def("tweak_wire_param", &AudioGraph::tweak_wire_param, py::arg("wire_id"), py::arg("param"));

    py::class_<AudioEngine>(m, "AudioEngine", py::module_local())
            .def(py::init<>())
            .def("startAudio", &AudioEngine::startAudio)
            .def("stopAudio", &AudioEngine::stopAudio)
            .def("get_graph", &AudioEngine::getAudioGraph)
            .def("get_audio_device", &AudioEngine::get_audio_device)
            .def("set_output_block", &AudioEngine::set_output_block, py::arg("node_index"),
                 py::arg("block_output_index"));


    py::class_<Mixer, Graph::Block, std::shared_ptr<Mixer>>(m, "Mixer");
    py::class_<Oscillator, Graph::Block, std::shared_ptr<Oscillator>>(m, "Oscillator");


    auto mono_mixer_class = py::class_<MonoMixer, Mixer, std::shared_ptr<MonoMixer>>(m, "MonoMixer", py::module_local())
            .def(py::init<>());
    mono_mixer_class.attr("INPUT_SIZE") = MonoMixer::static_input_size();
    mono_mixer_class.attr("OUTPUT_SIZE") = MonoMixer::static_output_size();
    mono_mixer_class.attr("OUT") = static_cast<int>(MonoMixer::OUT);

    auto stereo_mixer_class = py::class_<StereoMixer, Mixer, std::shared_ptr<StereoMixer>>(m, "StereoMixer",
                                                                                           py::module_local())
            .def(py::init<>());
    stereo_mixer_class.attr("INPUT_SIZE") = StereoMixer::static_input_size();
    stereo_mixer_class.attr("OUTPUT_SIZE") = StereoMixer::static_output_size();

    py::enum_<Product::ProductType>(m, "ProductType")
            .value("DUAL_CHANNELS", Product::ProductType::DUAL_CHANNELS)
            .value("CASCADE", Product::ProductType::CASCADE)
            .export_values();

    auto product_class = py::class_<Product, Graph::Block, std::shared_ptr<Product>>(m, "Product", py::module_local())
            .def(py::init<Product::ProductType>(),
                 py::arg("product_type") = Product::ProductType::DUAL_CHANNELS);
    product_class.attr("INPUT_SIZE") = Product::static_input_size();
    product_class.attr("OUTPUT_SIZE") = Product::static_output_size();
    product_class.attr("OUT1") = static_cast<int>(Product::OUT1);
    product_class.attr("OUT2") = static_cast<int>(Product::OUT2);

    // SineOsc bindings
    auto sine_osc_class = py::class_<SineOsc, Oscillator, std::shared_ptr<SineOsc>>(m, "SineOsc", py::module_local())
            .def(py::init<float, float>(),
                 py::arg("freq") = 110.0f,
                 py::arg("amplitude") = 1.0f);
    sine_osc_class.attr("INPUT_SIZE") = SineOsc::static_input_size();
    sine_osc_class.attr("OUTPUT_SIZE") = SineOsc::static_output_size();
    sine_osc_class.attr("FREQ") = static_cast<int>(SineOsc::FREQ);
    sine_osc_class.attr("AMP") = static_cast<int>(SineOsc::AMP);
    sine_osc_class.attr("OUT") = static_cast<int>(SineOsc::OUT);


}
