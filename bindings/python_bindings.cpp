#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "../src/core/audio_engine.h"
#include "../src/blocks/oscillators.h"
#include "../src/blocks/mixers.h"
#include "../src/blocks/envelopes.h"

namespace py = pybind11;
using namespace AAri;

PYBIND11_MODULE(AAri_cpp, m) {
    //Expose entt:entity type:
    py::class_<entt::entity>(m, "Entity");

    m.doc() = "AAri_cpp: Real-time audio engine backend"; // Module documentation
    py::enum_<ParamName>(m, "ParamName")
            .value("NONE", ParamName::NONE)
            .value("Input", ParamName::Input)
            .value("Freq", ParamName::Freq)
            .value("Phase", ParamName::Phase)
            .value("Amp", ParamName::Amp)
            .value("Attack", ParamName::Attack)
            .value("Decay", ParamName::Decay)
            .value("Sustain", ParamName::Sustain)
            .value("Release", ParamName::Release)
            .value("Out", ParamName::Out)
            .value("StereoOut", ParamName::StereoOut)
            .export_values();

    py::class_<Input1D>(m, "Input1D", py::module_local())
            .def(py::init<float, ParamName>(), py::arg("value") = 0.0f, py::arg("name") = ParamName::NONE)
            .def_readwrite("value", &Input1D::value)
            .def_readwrite("name", &Input1D::name);
    py::class_<Input2D>(m, "Input2D", py::module_local())
            .def(py::init<std::array<float, 2>, ParamName>(), py::arg("value") = std::array<float, 2>{0.0f, 0.0f},
                 py::arg("name") = ParamName::NONE)
            .def_readwrite("value", &Input2D::value)
            .def_readwrite("name", &Input2D::name);
    py::class_<Output1D>(m, "Output1D", py::module_local())
            .def(py::init<float, ParamName>(), py::arg("value") = 0.0f, py::arg("name") = ParamName::NONE)
            .def_readwrite("value", &Output1D::value)
            .def_readwrite("name", &Output1D::name);
    py::class_<Output2D>(m, "Output2D", py::module_local())
            .def(py::init<std::array<float, 2>, ParamName>(), py::arg("value") = std::array<float, 2>{0.0f, 0.0f},
                 py::arg("name") = ParamName::NONE)
            .def_readwrite("value", &Output2D::value)
            .def_readwrite("name", &Output2D::name);
    py::class_<InputND<2>>(m, "InputND2", py::module_local())
            .def(py::init<std::array<float, 2>, ParamName>(), py::arg("value") = std::array<float, 2>{0.0f, 0.0f},
                 py::arg("name") = ParamName::NONE)
            .def_readwrite("value", &InputND<2>::value)
            .def_readwrite("name", &InputND<2>::name);
    py::class_<InputND<4>>(m, "InputND4", py::module_local())
            .def(py::init<std::array<float, 4>, ParamName>(),
                 py::arg("value") = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f},
                 py::arg("name") = ParamName::NONE)
            .def_readwrite("value", &InputND<4>::value)
            .def_readwrite("name", &InputND<4>::name);
    py::class_<InputND<8>>(m, "InputND8", py::module_local())
            .def(py::init<std::array<float, 8>, ParamName>(),
                 py::arg("value") = std::array<float, 8>{0.0f, 0.0f, 0.0f, 0.0f,
                                                         0.0f, 0.0f, 0.0f, 0.0f},
                 py::arg("name") = ParamName::NONE)
            .def_readwrite("value", &InputND<8>::value)
            .def_readwrite("name", &InputND<8>::name);
    py::class_<InputND<16>>(m, "InputND16", py::module_local())
            .def(py::init<std::array<float, 16>, ParamName>(),
                 py::arg("value") = std::array<float, 16>{0.0f, 0.0f, 0.0f, 0.0f,
                                                          0.0f, 0.0f, 0.0f, 0.0f,
                                                          0.0f, 0.0f, 0.0f, 0.0f,
                                                          0.0f, 0.0f, 0.0f, 0.0f},
                 py::arg("name") = ParamName::NONE)
            .def_readwrite("value", &InputND<16>::value)
            .def_readwrite("name", &InputND<16>::name);

    py::class_<InputNDStereo<2>>(m, "InputNDStereo2", py::module_local())
            .def(py::init<std::array<float, 2>, std::array<float, 2>, ParamName>(),
                 py::arg("left") = std::array<float, 2>{0.0f, 0.0f},
                 py::arg("right") = std::array<float, 2>{0.0f, 0.0f},
                 py::arg("name") = ParamName::NONE)
            .def_readwrite("left", &InputNDStereo<2>::left)
            .def_readwrite("right", &InputNDStereo<2>::right)
            .def_readwrite("name", &InputNDStereo<2>::name);
    py::class_<InputNDStereo<4>>(m, "InputNDStereo4", py::module_local())
            .def(py::init<std::array<float, 4>, std::array<float, 4>, ParamName>(),
                 py::arg("left") = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f},
                 py::arg("right") = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f},
                 py::arg("name") = ParamName::NONE)
            .def_readwrite("left", &InputNDStereo<4>::left)
            .def_readwrite("right", &InputNDStereo<4>::right)
            .def_readwrite("name", &InputNDStereo<4>::name);
    py::class_<InputNDStereo<8>>(m, "InputNDStereo8", py::module_local())
            .def(py::init<std::array<float, 8>, std::array<float, 8>, ParamName>(),
                 py::arg("left") = std::array<float, 8>{0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                                        0.0f, 0.0f, 0.0f},
                 py::arg("right") = std::array<float, 8>{0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                                         0.0f, 0.0f, 0.0f},
                 py::arg("name") = ParamName::NONE)
            .def_readwrite("left", &InputNDStereo<8>::left)
            .def_readwrite("right", &InputNDStereo<8>::right)
            .def_readwrite("name", &InputNDStereo<8>::name);
    py::class_<OutputND<4>>(m, "OutputND4", py::module_local())
            .def(py::init<std::array<float, 4>, ParamName>(),
                 py::arg("value") = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f},
                 py::arg("name") = ParamName::NONE)
            .def_readwrite("value", &OutputND<4>::value)
            .def_readwrite("name", &OutputND<4>::name);

    auto wire = py::class_<Wire>(m, "Wire", py::module_local())
            .def_readonly("from_output", &Wire::from_output)
            .def_readonly("to_block", &Wire::to_block)
            .def_readonly("to_input", &Wire::to_input)
            .def_readonly("gain", &Wire::gain)
            .def_readonly("offset", &Wire::offset);

    //Expose wire transmit static functions:
    wire.def_static("transmit_1d_to_1d", &Wire::transmit_1d_to_1d, py::arg("registry"), py::arg("wire"));
    wire.def_static("broadcast_1d_to_2d", &Wire::broadcast_1d_to_2d, py::arg("registry"), py::arg("wire"));
    wire.def_static("transmit_to_mono_mixer_2", &Wire::transmit_to_mono_mixer<2>, py::arg("registry"), py::arg("wire"));
    wire.def_static("transmit_to_mono_mixer_4", &Wire::transmit_to_mono_mixer<4>, py::arg("registry"), py::arg("wire"));
    wire.def_static("transmit_to_mono_mixer_8", &Wire::transmit_to_mono_mixer<8>, py::arg("registry"), py::arg("wire"));
    wire.def_static("transmit_to_mono_mixer_16", &Wire::transmit_to_mono_mixer<16>, py::arg("registry"),
                    py::arg("wire"));

    wire.def_static("transmit_mono_to_stereo_mixer_2", &Wire::transmit_mono_to_stereo_mixer<2>, py::arg("registry"),
                    py::arg("wire"));
    wire.def_static("transmit_mono_to_stereo_mixer_4", &Wire::transmit_mono_to_stereo_mixer<4>, py::arg("registry"),
                    py::arg("wire"));
    wire.def_static("transmit_mono_to_stereo_mixer_8", &Wire::transmit_mono_to_stereo_mixer<8>, py::arg("registry"),
                    py::arg("wire"));
    wire.def_static("transmit_mono_to_stereo_mixer_16", &Wire::transmit_mono_to_stereo_mixer<16>, py::arg("registry"),
                    py::arg("wire"));

    wire.def_static("transmit_stereo_to_stereo_mixer_2", &Wire::transmit_stereo_to_stereo_mixer<2>, py::arg("registry"),
                    py::arg("wire"));
    wire.def_static("transmit_stereo_to_stereo_mixer_4", &Wire::transmit_stereo_to_stereo_mixer<4>, py::arg("registry"),
                    py::arg("wire"));
    wire.def_static("transmit_stereo_to_stereo_mixer_8", &Wire::transmit_stereo_to_stereo_mixer<8>, py::arg("registry"),
                    py::arg("wire"));
    wire.def_static("transmit_stereo_to_stereo_mixer_16", &Wire::transmit_stereo_to_stereo_mixer<16>,
                    py::arg("registry"),
                    py::arg("wire"));


    py::enum_<BlockType>(m, "BlockType")
            .value("NONE", BlockType::NONE)
            .value("SineOsc", BlockType::SineOsc)
            .value("TriOsc", BlockType::TriOsc)
            .value("SquareOsc", BlockType::SquareOsc)
            .value("SawOsc", BlockType::SawOsc)
            .value("Product", BlockType::Product)
            .value("Sum", BlockType::Sum)
            .value("Constant", BlockType::Constant)
            .value("MonoMixer", BlockType::MonoMixer)
            .value("StereoMixer", BlockType::StereoMixer)
            .export_values();

    py::class_<WiresToBlock>(m, "WiresToBlock", py::module_local())
            .def_readonly("input_wire_ids", &WiresToBlock::input_wire_ids);

    py::class_<Block>(m, "Block", py::module_local())
            .def_readonly("inputIds", &Block::inputIds)
            .def_readonly("outputIds", &Block::outputIds)
            .def_readonly("type", &Block::type)
            .def_readonly("topo_sort_index", &Block::topo_sort_index);

    py::class_<InputExpansion>(m, "InputExpansion", py::module_local())
            .def_readonly("inputId", &InputExpansion::inputIds);
    py::class_<OutputExpansion>(m, "OutputExpansion", py::module_local())
            .def_readonly("outputId", &OutputExpansion::outputIds);

//    py::enum_<WireTransform>(m, "WireTransform")
//            .value("NONE", WireTransform::NONE)
//            .value("EXPAND", WireTransform::EXPAND)
//            .value("STEREO_PAN", WireTransform::STEREO_PAN)
//            .export_values();

//    py::class_<Wire>(m, "Wire", py::module_local())
//            .def_readonly("input", &Wire::in)
//            .def_readonly("output", &Wire::out)
//            .def_readonly("in_index", &Wire::in_index)
//            .def_readonly("width", &Wire::width)
//            .def_readonly("out_index", &Wire::out_index)
//            .def_readonly("gain", &Wire::gain)
//            .def_readonly("offset", &Wire::offset)
//            .def_readonly("transform", &Wire::transform)
//            .def_readonly("wire_transform_param", &Wire::wire_transform_param);
//
//    py::class_<deprecated_Graph::Block, std::shared_ptr<deprecated_Graph::Block>>(m, "Block", py::module_local())
//            .def_property_readonly("id", &deprecated_Graph::Block::id)
//            .def_property_readonly("name", &deprecated_Graph::Block::name)
//            .def_property_readonly("input_size", &deprecated_Graph::Block::input_size)
//            .def_property_readonly("output_size", &deprecated_Graph::Block::output_size)
//            .def_readonly("last_processed_time", &deprecated_Graph::Block::last_processed_time)
//            .def_property_readonly("wires", &deprecated_Graph::Block::py_get_input_wires);


//    py::class_<AudioGraph>(m, "AudioGraph", py::module_local())
//            .def(py::init([](std::shared_ptr<AudioEngine> engine) {
//                return new AudioGraph(engine.get());
//            }))
//            .def("add_block", &AudioGraph::add_block, py::arg("block"))
//            .def("remove_block", &AudioGraph::remove_block, py::arg("block_id"))
//            .def("connect_wire", &AudioGraph::connect_wire, py::arg("in_block_id"), py::arg("out_block_id"),
//                 py::arg("in_index"), py::arg("width"), py::arg("out_index"),
//                 py::arg("gain") = 1.0f,
//                 py::arg("offset") = 0.0f,
//                 py::arg("transform") = WireTransform::NONE,
//                 py::arg("wire_transform_param") = 0.0f)
//            .def("disconnect_wire", &AudioGraph::disconnect_wire, py::arg("wire_id"),
//                 py::arg("out_block_id") = py::none())
//            .def("has_block", &AudioGraph::has_block, py::arg("block_id"))
//            .def("get_all_blocks", &AudioGraph::py_get_all_blocks)
//            .def("get_topological_order", &AudioGraph::py_get_topological_order)
//            .def("get_block_inputs", &AudioGraph::py_get_block_inputs, py::arg("block_id"), py::arg("input_index"),
//                 py::arg("width"))
//            .def("set_block_inputs", &AudioGraph::py_set_block_inputs, py::arg("block_id"), py::arg("input_index"),
//                 py::arg("input"))
//            .def("get_block_outputs", &AudioGraph::py_get_block_outputs, py::arg("block_id"),
//                 py::arg("output_index"),
//                 py::arg("width"))
//            .def("tweak_wire_gain", &AudioGraph::tweak_wire_gain, py::arg("wire_id"), py::arg("gain"))
//            .def("tweak_wire_offset", &AudioGraph::tweak_wire_offset, py::arg("wire_id"), py::arg("offset"))
//            .def("tweak_wire_param", &AudioGraph::tweak_wire_param, py::arg("wire_id"), py::arg("param"));
//
//    py::class_<AudioEngine>(m, "AudioEngine", py::module_local())
//            .def(py::init<>())
//            .def("startAudio", &AudioEngine::startAudio)
//            .def("stopAudio", &AudioEngine::stopAudio)
//            .def("get_graph", &AudioEngine::getAudioGraph)
//            .def("get_audio_device", &AudioEngine::get_audio_device)
//            .def("set_output_block", &AudioEngine::set_output_block, py::arg("node_index"),
//                 py::arg("block_output_index"));
//
//
//    py::class_<Mixer, deprecated_Graph::Block, std::shared_ptr<Mixer>>(m, "Mixer");
//    //py::class_<Oscillator, deprecated_Graph::Block, std::shared_ptr<Oscillator>>(m, "Oscillator");
//
//
//    auto mono_mixer_class = py::class_<MonoMixer, Mixer, std::shared_ptr<MonoMixer>>(m, "MonoMixer", py::module_local())
//            .def(py::init<>());
//    mono_mixer_class.attr("INPUT_SIZE") = MonoMixer::static_input_size();
//    mono_mixer_class.attr("OUTPUT_SIZE") = MonoMixer::static_output_size();
//    mono_mixer_class.attr("OUT") = static_cast<int>(MonoMixer::OUT);
//
//    auto stereo_mixer_class = py::class_<StereoMixer, Mixer, std::shared_ptr<StereoMixer>>(m, "StereoMixer",
//                                                                                           py::module_local())
//            .def(py::init<>());
//    stereo_mixer_class.attr("INPUT_SIZE") = StereoMixer::static_input_size();
//    stereo_mixer_class.attr("OUTPUT_SIZE") = StereoMixer::static_output_size();
//
//    py::enum_<Product::ProductType>(m, "ProductType")
//            .value("DUAL_CHANNELS", Product::ProductType::DUAL_CHANNELS)
//            .value("CASCADE", Product::ProductType::CASCADE)
//            .export_values();
//
//    auto product_class = py::class_<Product, deprecated_Graph::Block, std::shared_ptr<Product>>(m, "Product",
//                                                                                                py::module_local())
//            .def(py::init<Product::ProductType>(),
//                 py::arg("product_type") = Product::ProductType::DUAL_CHANNELS);
//    product_class.attr("INPUT_SIZE") = Product::static_input_size();
//    product_class.attr("OUTPUT_SIZE") = Product::static_output_size();
//    product_class.attr("OUT1") = static_cast<int>(Product::OUT1);
//    product_class.attr("OUT2") = static_cast<int>(Product::OUT2);

    // SineOsc bindings
//    auto sine_osc_class = py::class_<SineOsc, Oscillator, std::shared_ptr<SineOsc>>(m, "SineOsc", py::module_local())
//            .def(py::init<float, float>(),
//                 py::arg("freq") = 110.0f,
//                 py::arg("amplitude") = 1.0f);
//    sine_osc_class.attr("INPUT_SIZE") = SineOsc::static_input_size();
//    sine_osc_class.attr("OUTPUT_SIZE") = SineOsc::static_output_size();
//    sine_osc_class.attr("FREQ") = static_cast<int>(SineOsc::FREQ);
//    sine_osc_class.attr("AMP") = static_cast<int>(SineOsc::AMP);
//    sine_osc_class.attr("OUT") = static_cast<int>(SineOsc::OUT);


}
