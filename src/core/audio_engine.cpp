
#define MA_IMPLEMENTATION
#include "audio_engine.h"
#include <iostream>

AudioEngine::AudioEngine(size_t sample_rate, size_t buffer_size) : clock_seconds(0), outputNodeIndex(0), outputChannelStart(0) {

    // Open audio device
    deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;
    deviceConfig.playback.channels = 2;
    deviceConfig.sampleRate        = sample_rate;
    deviceConfig.dataCallback      = audio_callback;
    deviceConfig.pUserData         = this;
    deviceConfig.periodSizeInFrames = buffer_size;
    audioGraph = std::make_shared<Graph::AudioGraph>(get_audio_device());

}

AudioEngine::~AudioEngine() {
    ma_device_uninit(&device);
}

void AudioEngine::startAudio() {
    ma_device_start(&device);
    clock_seconds = 0;
}

void AudioEngine::stopAudio() {
    ma_device_stop(&device);
}

void AudioEngine::audio_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount) {
    AudioEngine *engine = static_cast<AudioEngine *>(pDevice->pUserData);

    if (engine->audioGraph == nullptr || engine->outputNodeIndex == 0) {
        return;
    }
    Graph::Block *outputBlock;
    if (!engine->audioGraph->get_block(engine->outputNodeIndex, &outputBlock)) {
        return;
    }
    auto *buffer = (float *) pOutput;
    const float sample_freq = (float) pDevice->sampleRate;
    const double seconds_per_sample = 1.0 / sample_freq;
    auto block_output = outputBlock->outputs();
    const auto output_size = (size_t) outputBlock->output_size();
    const auto offset = engine->outputChannelStart;

    for (size_t i = 0; i < 2* frameCount; i += 2) {
        engine->clock_seconds += seconds_per_sample;
        engine->audioGraph->process({sample_freq, engine->clock_seconds});
        buffer[i] = block_output[offset];
        buffer[i + 1] = offset + 1 < output_size ? block_output[offset + 1] : buffer[i];

    }
}

void AudioEngine::set_output_block(size_t node_index, size_t block_output_index) {
    Graph::Block *outputBlock = nullptr;
    if (!audioGraph->get_block(node_index, &outputBlock)) {
        throw std::runtime_error("Invalid output node index : node doesn't exist on the graph");
    }
    if (block_output_index >= outputBlock->output_size()) {
        throw std::runtime_error("Invalid output channel start : block doesn't have enough outputs");
    }
    //TODO LOCK !
    outputNodeIndex = node_index;
    outputChannelStart = block_output_index;
    //TODO UNLOCK !

}

// FOR SDL2
int main(int argc, char *argv[]) {
    return 0;
}
