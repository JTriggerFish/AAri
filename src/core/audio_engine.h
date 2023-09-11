
#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H
#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <mutex>
#include "graph.h"
#include <memory>

class AudioEngine {
public:
    AudioEngine();

    ~AudioEngine();

    void startAudio();

    void stopAudio();

    std::shared_ptr<Graph::AudioGraph> getAudioGraph() {
        return audioGraph;
    }

    void set_output_block(size_t node_index, size_t block_output_index);

    SDL_AudioDeviceID get_audio_device() const {
        return audioDevice;
    }

private:
    static void audioCallback(void *userdata, Uint8 *stream, int len);

    double clock_seconds;

    SDL_AudioDeviceID audioDevice;
    SDL_AudioSpec audioSpec;
    std::shared_ptr<Graph::AudioGraph> audioGraph;
    size_t outputNodeIndex;
    size_t outputChannelStart;
};

#endif // AUDIO_ENGINE_H
