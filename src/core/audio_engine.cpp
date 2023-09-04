
#include "audio_engine.h"
#include <iostream>

AudioEngine::AudioEngine() : clock_seconds(0), outputNodeIndex(0), outputChannelStart(0) {
    // Initialize SDL2 Audio
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "Failed to initialize SDL2 Audio: " << SDL_GetError() << std::endl;
        return;
    }

    // Audio spec initialization
    SDL_zero(audioSpec);
    audioSpec.freq = 48000;
    audioSpec.format = AUDIO_F32;
    audioSpec.channels = 2;
    audioSpec.samples = 1024;
    audioSpec.callback = AudioEngine::audioCallback;
    audioSpec.userdata = this;

    // Open audio device
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &audioSpec, NULL, 0);
    if (audioDevice == 0) {
        std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
    }
    audioGraph = std::make_shared<Graph::AudioGraph>(get_audio_device());

}

AudioEngine::~AudioEngine() {
    SDL_CloseAudioDevice(audioDevice);
    SDL_Quit();
}

void AudioEngine::startAudio() {
    SDL_PauseAudioDevice(audioDevice, 0);
    clock_seconds = 0;
}

void AudioEngine::stopAudio() {
    SDL_PauseAudioDevice(audioDevice, 1);
}

void AudioEngine::audioCallback(void *userdata, Uint8 *stream, int _len) {
    AudioEngine *engine = static_cast<AudioEngine *>(userdata);
    auto len = size_t(_len);

    if (engine->audioGraph == nullptr || engine->outputNodeIndex == 0) {
        return;
    }
    Graph::Block *outputBlock;
    if (!engine->audioGraph->get_block(engine->outputNodeIndex, &outputBlock)) {
        return;
    }
    auto *buffer = (float *) stream;
    const float sample_freq = engine->audioSpec.freq;
    const double seconds_per_sample = 1.0 / engine->audioSpec.freq;
    auto block_output = outputBlock->outputs();
    const auto output_size = (size_t) outputBlock->output_size();
    const auto offset = engine->outputChannelStart;

    for (size_t i = 0; i < len / sizeof(float); i += 2) {
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
    SDL_LockAudioDevice(audioDevice);
    outputNodeIndex = node_index;
    outputChannelStart = block_output_index;
    SDL_UnlockAudioDevice(audioDevice);

}

// FOR SDL2
int main(int argc, char *argv[]) {
    return 0;
}
