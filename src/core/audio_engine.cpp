
#define MA_IMPLEMENTATION

#include "audio_engine.h"
#include <iostream>

AudioEngine::AudioEngine(ma_uint32 sample_rate, ma_uint32 buffer_size) : clock_seconds(0), _outputNodeIndex(0),
                                                                         _outputChannelStart(0) {

    // Open audio device
    _deviceConfig = ma_device_config_init(ma_device_type_playback);
    _deviceConfig.playback.format = ma_format_f32;
    _deviceConfig.playback.channels = 2;
    _deviceConfig.sampleRate = sample_rate;
    _deviceConfig.dataCallback = audio_callback;
    _deviceConfig.pUserData = this;
    _deviceConfig.periodSizeInFrames = buffer_size;

    if (ma_device_init(NULL, &_deviceConfig, &_device) != MA_SUCCESS) {
        throw std::runtime_error("Failed to open playback device.");
    }
    _audioGraph = std::make_shared<Graph::AudioGraph>(this);

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
    AudioEngine *engine = static_cast<AudioEngine *>(pDevice->pUserData);
    auto guard = engine->lock_till_function_returns();

    if (engine->_audioGraph == nullptr || engine->_outputNodeIndex == 0) {
        return;
    }
    Graph::Block *outputBlock;
    if (!engine->_audioGraph->get_block(engine->_outputNodeIndex, &outputBlock)) {
        return;
    }
    auto *buffer = (float *) pOutput;
    const float sample_freq = (float) pDevice->sampleRate;
    const double seconds_per_sample = 1.0 / sample_freq;
    auto block_output = outputBlock->outputs();
    const auto output_size = (size_t) outputBlock->output_size();
    const auto offset = engine->_outputChannelStart;

    for (size_t i = 0; i < 2 * frameCount; i += 2) {
        engine->clock_seconds += seconds_per_sample;
        engine->_audioGraph->process({sample_freq, engine->clock_seconds});
        buffer[i] = block_output[offset];
        buffer[i + 1] = offset + 1 < output_size ? block_output[offset + 1] : buffer[i];

    }
}

void AudioEngine::set_output_block(size_t node_index, size_t block_output_index) {
    Graph::Block *outputBlock = nullptr;
    if (!_audioGraph->get_block(node_index, &outputBlock)) {
        throw std::runtime_error("Invalid output node index : node doesn't exist on the graph");
    }
    if (block_output_index >= outputBlock->output_size()) {
        throw std::runtime_error("Invalid output channel start : block doesn't have enough outputs");
    }
    auto guard = lock_till_function_returns();
    _outputNodeIndex = node_index;
    _outputChannelStart = block_output_index;

}