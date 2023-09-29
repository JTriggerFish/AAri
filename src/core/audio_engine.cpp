
#define MA_IMPLEMENTATION

#include "audio_engine.h"
#include "graph.h"
#include "blocks.h"
#include "parameters.h"
#include <iostream>

using namespace AAri;

AudioEngine::AudioEngine(ma_uint32 sample_rate, ma_uint32 buffer_size) : clock_seconds(0),
                                                                         _output_id(entt::null) {

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
    //printf("Audio engine destroyed\n");
}

void AudioEngine::startAudio() {
    ma_device_start(&_device);
    clock_seconds = 0;
}

void AudioEngine::stopAudio() {
    ma_device_stop(&_device);
}

void AudioEngine::audio_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount) {
    auto *engine = static_cast<AudioEngine *>(pDevice->pUserData);
    auto [registry, guard] = engine->get_graph_registry();

    auto *buffer = (float *) pOutput;
    const auto sample_freq = (float) pDevice->sampleRate;
    const float seconds_per_sample = 1.0f / sample_freq;

    float *output;
    const size_t width = engine->_output_width;
    if (width == 1)
        output = &(registry.get<Output1D>(engine->_output_id).value);
    else if (width == 2)
        output = &(registry.get<Output2D>(engine->_output_id).value[0]);
    else
        throw std::runtime_error("Output width not supported");

    for (size_t i = 0; i < 2 * frameCount; i += 2) {
        engine->clock_seconds += seconds_per_sample;
        engine->_graph.process({sample_freq, seconds_per_sample, engine->clock_seconds});

        buffer[i] = output[0];
        buffer[i + 1] = width == 2 ? output[1] : output[0];
    }
}

void AudioEngine::set_output(entt::entity output_id, size_t output_width) {
    auto guard = lock_till_function_returns();
    _output_id = output_id;
    _output_width = output_width;
}
