
#define MA_IMPLEMENTATION

#include "audio_engine.h"
#include "blocks.h"
#include "graph.h"
#include "parameters.h"
#include <iostream>

using namespace AAri;

AudioEngine::AudioEngine(ma_uint32 sample_rate, ma_uint32 buffer_size)
        : clock_seconds(0), _output_id(entt::null), _output_width(0) {

    // Open audio device
    _deviceConfig = ma_device_config_init(ma_device_type_playback);
    _deviceConfig.playback.format = ma_format_f32;
    _deviceConfig.playback.channels = 2;
    _deviceConfig.sampleRate = sample_rate;
    _deviceConfig.dataCallback = audio_callback;
    _deviceConfig.pUserData = this;
    _deviceConfig.periodSizeInFrames = buffer_size;

    if (ma_device_init(nullptr, &_deviceConfig, &_device) != MA_SUCCESS) {
        throw std::runtime_error("Failed to open playback device.");
    }
}

AudioEngine::~AudioEngine() {
    ma_device_uninit(&_device);
    ma_device_stop(&_device);
    // printf("Audio engine destroyed\n");
}

void AudioEngine::startAudio() {
    ma_device_start(&_device);
    clock_seconds = 0.0;
}

void AudioEngine::stopAudio() { ma_device_stop(&_device); }

void AudioEngine::audio_callback(ma_device *pDevice, void *pOutput,
                                 const void *pInput, ma_uint32 frameCount) {
    auto *engine = static_cast<AudioEngine *>(pDevice->pUserData);
    auto [registry, guard] = engine->get_graph_registry();

    auto *buffer = (float *) pOutput;
    const auto sample_freq = (float) pDevice->sampleRate;
    const float seconds_per_sample = 1.0f / sample_freq;

    float *output;
    const size_t width = engine->_output_width;
    if (width == 0)
        return;
    if (width == 1)
        output = &(registry.get<Output1D>(engine->_output_id).value);
    else if (width == 2)
        output = &(registry.get<Output2D>(engine->_output_id).value[0]);
    else
        throw std::runtime_error("Invalid output width: " + std::to_string(width));

    for (size_t i = 0; i < 2 * frameCount; i += 2) {
        engine->clock_seconds += seconds_per_sample;
        engine->_graph.process(
                {sample_freq, seconds_per_sample, engine->clock_seconds});

        buffer[i] = output[0];
        buffer[i + 1] = width == 2 ? output[1] : output[0];
    }
}

void AudioEngine::set_output(entt::entity output_id, size_t output_width) {
    auto guard = lock_till_function_returns();
    _output_id = output_id;
    _output_width = output_width;
}

entt::entity AudioEngine::add_wire(entt::entity from_block,
                                   entt::entity to_block, entt::entity from_output,
                                   entt::entity to_input, TransmitFunc transmitFunc,
                                   float gain, float offset) {
    auto [registry, lock] = get_graph_registry();
    auto entity =
            Wire::create(registry, from_block, to_block,
                         from_output,
                         to_input,
                         transmitFunc, gain, offset);

    // Need to do a topological sort of the graph
    _graph.toposort_blocks();
    return entity;
}

entt::entity AudioEngine::add_wire_to_mixer(entt::entity from_block,
                                            entt::entity to_block, entt::entity from_output,
                                            size_t to_mixer_input_index, TransmitFunc transmitFunc,
                                            float gain, float offset) {
    //For mixers we abuse the system a bit and store an index as an entity
    return add_wire(from_block, to_block, from_output, (entt::entity) to_mixer_input_index,
                    transmitFunc, gain, offset);
}

void AudioEngine::remove_wire(entt::entity wire_id) {
    auto [registry, guard] = get_graph_registry();
    Wire::destroy(registry, wire_id);

    // Need to do a topological sort of the graph
    _graph.toposort_blocks();
}

void AudioEngine::remove_block(entt::entity block_id) {
    auto [registry, guard] = get_graph_registry();
    // First remove all the wires connected to this block:
    auto view = registry.view<Wire>();
    for (auto entity: view) {
        auto &wire = view.get<Wire>(entity);
        if (wire.from_block == block_id || wire.to_block == block_id) {
            Wire::destroy(registry, entity);
        }
    }
    Block::destroy(registry, block_id);

    // Need to do a topological sort of the graph
    _graph.toposort_blocks();
}

Block AudioEngine::view_block(entt::entity block_id) {
    return _graph.registry.get<Block>(block_id);
}

std::vector<entt::entity>
AudioEngine::get_wires_to_block(entt::entity block_id) {
    std::vector<entt::entity> wires;
    auto view = _graph.registry.view<Wire>();
    for (auto entity: view) {
        auto &wire = view.get<Wire>(entity);
        if (wire.to_block == block_id) {
            wires.push_back(entity);
        }
    }
    return wires;
}

std::vector<entt::entity>
AudioEngine::get_wires_from_block(entt::entity block_id) {
    std::vector<entt::entity> wires;
    auto view = _graph.registry.view<Wire>();
    for (auto entity: view) {
        auto &wire = view.get<Wire>(entity);
        if (wire.from_block == block_id) {
            wires.push_back(entity);
        }
    }
    return wires;
}

std::optional<entt::entity>
AudioEngine::get_wire_to_input(entt::entity input_id) {
    auto view = _graph.registry.view<Wire>();
    for (auto entity: view) {
        auto &wire = view.get<Wire>(entity);
        if (wire.to_input == input_id) {
            return entity;
        }
    }
    return std::nullopt;
}

std::vector<entt::entity>
AudioEngine::get_wires_from_output(entt::entity output_id) {
    std::vector<entt::entity> wires;
    auto view = _graph.registry.view<Wire>();
    for (auto entity: view) {
        auto &wire = view.get<Wire>(entity);
        if (wire.from_output == output_id) {
            wires.push_back(entity);
        }
    }
    return wires;
}

Wire AudioEngine::view_wire(entt::entity wire_id) {
    return _graph.registry.get<Wire>(wire_id);
}

void AudioEngine::tweak_wire_gain(entt::entity wire_id, float gain) {
    auto [registry, guard] = get_graph_registry();
    auto &wire = registry.get<Wire>(wire_id);
    wire.gain = gain;
}

void AudioEngine::tweak_wire_offset(entt::entity wire_id, float offset) {
    auto [registry, guard] = get_graph_registry();
    auto &wire = registry.get<Wire>(wire_id);
    wire.offset = offset;
}
