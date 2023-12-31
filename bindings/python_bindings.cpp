#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>

#include "../src/core/audio_engine.h"
#include "../src/blocks/oscillators.h"
#include "../src/blocks/mixers.h"
#include "../src/blocks/envelopes.h"

namespace py = pybind11;
using namespace AAri;

PYBIND11_MODULE(AAri_cpp, m) {
        py::class_<AudioContext>(m, "AudioContext")
                        .def_readonly("sample_freq", &AudioContext::sample_freq)
                        .def_readonly("dt", &AudioContext::dt)
                        .def_readonly("clock", &AudioContext::clock);

        py::class_<entt::entity>(m, "Entity");
        py::class_<entt::registry>(m, "Registry");

        m.doc() = "AAri_cpp: Real-time audio engine backend"; // Module documentation

        py::class_<InputOutput>(m, "InputOutput", py::module_local());

        py::class_<Input1D, InputOutput>(m, "Input1D", py::module_local())
                        .def(py::init<float>(), py::arg("value") = 0.0f)
                        .def_readonly("value", &Input1D::value);
        py::class_<Output1D, InputOutput>(m, "Output1D", py::module_local())
                        .def(py::init<float>(), py::arg("value") = 0.0f)
                        .def_readonly("value", &Output1D::value);
        py::class_<InputND<2>, InputOutput>(m, "InputND2", py::module_local())
                        .def(py::init<std::array<float, 2>>(), py::arg("value") = std::array<float, 2>{0.0f, 0.0f})
                        .def_readonly("value", &InputND<2>::value);
        py::class_<InputND<4>, InputOutput>(m, "InputND4", py::module_local())
                        .def(py::init<std::array<float, 4>>(),
                             py::arg("value") = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f})
                        .def_readonly("value", &InputND<4>::value);
        py::class_<InputND<8>, InputOutput>(m, "InputND8", py::module_local())
                        .def(py::init<std::array<float, 8>>(),
                             py::arg("value") = std::array<float, 8>{
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f
                             })
                        .def_readonly("value", &InputND<8>::value);
        py::class_<InputND<16>, InputOutput>(m, "InputND16", py::module_local())
                        .def(py::init<std::array<float, 16>>(),
                             py::arg("value") = std::array<float, 16>{
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                             })
                        .def_readonly("value", &InputND<16>::value);
        py::class_<InputND<32>, InputOutput>(m, "InputND32", py::module_local())
                        .def(py::init<std::array<float, 32>>(),
                             py::arg("value") = std::array<float, 32>{
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f, 0.0f,
                             })
                        .def_readonly("value", &InputND<32>::value);


        py::class_<InputNDStereo<2>, InputOutput>(
                                m, "InputNDStereo2", py::module_local())
                        .def(py::init<std::array<float, 2>, std::array<float, 2>>(),
                             py::arg("left") = std::array<float, 2>{0.0f, 0.0f},
                             py::arg("right") = std::array<float, 2>{0.0f, 0.0f})
                        .def_readonly("left", &InputNDStereo<2>::left)
                        .def_readonly("right", &InputNDStereo<2>::right);
        py::class_<InputNDStereo<4>, InputOutput>(m, "InputNDStereo4", py::module_local())
                        .def(py::init<std::array<float, 4>, std::array<float, 4>>(),
                             py::arg("left") = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f},
                             py::arg("right") = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f})
                        .def_readonly("left", &InputNDStereo<4>::left)
                        .def_readonly("right", &InputNDStereo<4>::right);
        py::class_<InputNDStereo<8>, InputOutput>(m, "InputNDStereo8", py::module_local())
                        .def(py::init<std::array<float, 8>, std::array<float, 8>>(),
                             py::arg("left") = std::array<float, 8>{
                                     0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f
                             },
                             py::arg("right") = std::array<float, 8>{
                                     0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                     0.0f, 0.0f, 0.0f
                             })
                        .def_readwrite("left", &InputNDStereo<8>::left)
                        .def_readwrite("right", &InputNDStereo<8>::right);
        py::class_<OutputND<2>, InputOutput>(m, "OutputND2", py::module_local())
                        .def(py::init<std::array<float, 2>>(),
                             py::arg("value") = std::array<float, 2>{0.0f, 0.0f})
                        .def_readonly("value", &OutputND<2>::value);
        py::class_<OutputND<4>, InputOutput>(m, "OutputND4", py::module_local())
                        .def(py::init<std::array<float, 4>>(),
                             py::arg("value") = std::array<float, 4>{0.0f, 0.0f, 0.0f, 0.0f})
                        .def_readonly("value", &OutputND<4>::value);

        auto wire = py::class_<Wire>(m, "Wire", py::module_local())
                        .def_readonly("from_block", &Wire::from_block)
                        .def_readonly("from_output", &Wire::from_output)
                        .def_readonly("to_block", &Wire::to_block)
                        .def_readonly("to_input", &Wire::to_input)
                        .def_readonly("gain", &Wire::gain)
                        .def_readonly("offset", &Wire::offset);

        //Expose wire transmit static functions:
        wire.def_static("transmit_1d_to_1d", &Wire::transmit_1d_to_1d, py::arg("registry"), py::arg("wire"));
        wire.def_static("broadcast_1d_to_2d", &Wire::broadcast_1d_to_Nd<2>, py::arg("registry"), py::arg("wire"));
        wire.def_static("broadcast_1d_to_4d", &Wire::broadcast_1d_to_Nd<4>, py::arg("registry"), py::arg("wire"));
        wire.def_static("broadcast_1d_to_8d", &Wire::broadcast_1d_to_Nd<8>, py::arg("registry"), py::arg("wire"));
        wire.def_static("broadcast_1d_to_16d", &Wire::broadcast_1d_to_Nd<16>, py::arg("registry"), py::arg("wire"));
        wire.def_static("broadcast_1d_to_32d", &Wire::broadcast_1d_to_Nd<32>, py::arg("registry"), py::arg("wire"));
        wire.def_static("transmit_to_mono_mixer_2", &Wire::transmit_to_mono_mixer<2>, py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_to_mono_mixer_4", &Wire::transmit_to_mono_mixer<4>, py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_to_mono_mixer_8", &Wire::transmit_to_mono_mixer<8>, py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_to_mono_mixer_16", &Wire::transmit_to_mono_mixer<16>, py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_to_mono_mixer_32", &Wire::transmit_to_mono_mixer<32>, py::arg("registry"),
                        py::arg("wire"));

        wire.def_static("transmit_mono_to_stereo_mixer_2", &Wire::transmit_mono_to_stereo_mixer<2>, py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_mono_to_stereo_mixer_4", &Wire::transmit_mono_to_stereo_mixer<4>, py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_mono_to_stereo_mixer_8", &Wire::transmit_mono_to_stereo_mixer<8>, py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_mono_to_stereo_mixer_16", &Wire::transmit_mono_to_stereo_mixer<16>,
                        py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_mono_to_stereo_mixer_32", &Wire::transmit_mono_to_stereo_mixer<32>,
                        py::arg("registry"),
                        py::arg("wire"));

        wire.def_static("transmit_stereo_to_stereo_mixer_2", &Wire::transmit_stereo_to_stereo_mixer<2>,
                        py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_stereo_to_stereo_mixer_4", &Wire::transmit_stereo_to_stereo_mixer<4>,
                        py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_stereo_to_stereo_mixer_8", &Wire::transmit_stereo_to_stereo_mixer<8>,
                        py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_stereo_to_stereo_mixer_16", &Wire::transmit_stereo_to_stereo_mixer<16>,
                        py::arg("registry"),
                        py::arg("wire"));
        wire.def_static("transmit_stereo_to_stereo_mixer_32", &Wire::transmit_stereo_to_stereo_mixer<32>,
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
                        .value("StereoMixer", BlockType::StereoMixer);

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

        py::class_<IGraphRegistry>(m, "IGraphRegistry", py::module_local());

        py::class_<AudioEngine, IGraphRegistry>(m, "AudioEngine", py::module_local())
                        .def(py::init<>())
                        .def("startAudio", &AudioEngine::startAudio)
                        .def("stopAudio", &AudioEngine::stopAudio)
                        .def("add_wire", &AudioEngine::add_wire, py::arg("from_block"), py::arg("to_block"),
                             py::arg("from_output"),
                             py::arg("to_input"), py::arg("transmitFunc"), py::arg("gain") = 1.0f,
                             py::arg("offset") = 0.0f)
                        .def("add_wire_to_mixer", &AudioEngine::add_wire_to_mixer, py::arg("from_block"),
                             py::arg("to_block"),
                             py::arg("from_output"), py::arg("to_mixer_input_index"), py::arg("transmitFunc"),
                             py::arg("gain") = 1.0f, py::arg("offset") = 0.0f)
                        .def("remove_wire", &AudioEngine::remove_wire, py::arg("wire_id"))
                        .def("remove_block", &AudioEngine::remove_block, py::arg("block_id"))
                        .def("tweak_wire_gain", &AudioEngine::tweak_wire_gain, py::arg("wire_id"), py::arg("gain"))
                        .def("tweak_wire_offset", &AudioEngine::tweak_wire_offset, py::arg("wire_id"),
                             py::arg("offset"))
                        .def("set_output_ref", &AudioEngine::set_output_ref, py::arg("output_id"),
                             py::arg("output_width"))
                        .def("view_block", &AudioEngine::view_block, py::arg("block_id"))
                        .def("view_wire", &AudioEngine::view_wire, py::arg("wire_id"))
                        .def("get_wires_to_block", &AudioEngine::get_wires_to_block, py::arg("block_id"))
                        .def("get_wires_from_block", &AudioEngine::get_wires_from_block, py::arg("block_id"))
                        .def("get_wire_to_input", &AudioEngine::get_wire_to_input, py::arg("input_id"))
                        .def("get_wire_from_output", &AudioEngine::get_wires_from_output, py::arg("output_id"))
                        .def("get_blocks", &AudioEngine::get_blocks)
                        .def("view_block_io", &AudioEngine::view_block_io, py::arg("block_id"))
                        .def("get_output_ref", &AudioEngine::get_output_ref)
                        .def("set_input_1d", &AudioEngine::set_input_1d, py::arg("input_id"), py::arg("value"))
                        .def("set_input_2d", &AudioEngine::set_input_Nd<2>, py::arg("input_id"), py::arg("value"))
                        .def("set_input_4d", &AudioEngine::set_input_Nd<4>, py::arg("input_id"), py::arg("value"));

        // Mixers
        py::class_<MonoMixer<2>>(m, "MonoMixer2", py::module_local())
                        .def_static("create", &MonoMixer<2>::create);
        py::class_<MonoMixer<4>>(m, "MonoMixer4", py::module_local())
                        .def_static("create", &MonoMixer<4>::create);
        py::class_<MonoMixer<8>>(m, "MonoMixer8", py::module_local())
                        .def_static("create", &MonoMixer<8>::create);
        py::class_<MonoMixer<16>>(m, "MonoMixer16", py::module_local())
                        .def_static("create", &MonoMixer<16>::create);
        py::class_<MonoMixer<32>>(m, "MonoMixer32", py::module_local())
                        .def_static("create", &MonoMixer<32>::create);
        py::class_<StereoMixer<2>>(m, "StereoMixer2", py::module_local())
                        .def_static("create", &StereoMixer<2>::create);
        py::class_<StereoMixer<4>>(m, "StereoMixer4", py::module_local())
                        .def_static("create", &StereoMixer<4>::create);
        py::class_<StereoMixer<8>>(m, "StereoMixer8", py::module_local())
                        .def_static("create", &StereoMixer<8>::create);
        py::class_<StereoMixer<16>>(m, "StereoMixer16", py::module_local())
                        .def_static("create", &StereoMixer<16>::create);
        py::class_<StereoMixer<32>>(m, "StereoMixer32", py::module_local())
                        .def_static("create", &StereoMixer<32>::create);

        //Oscillators
        py::class_<SineOsc>(m, "SineOsc", py::module_local())
                        .def_static("create", &SineOsc::create, py::arg("engine"), py::arg("freq") = 440.0f,
                                    py::arg("amp") = 1.0f);
}
