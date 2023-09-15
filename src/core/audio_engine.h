
#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H


#include "../miniaudio.h"
#include <mutex>
#include "graph.h"
#include <memory>

class AudioEngine {
public:
    AudioEngine(size_t sample_rate = 48000, size_t buffer_size = 512);

    ~AudioEngine();

    void startAudio();

    void stopAudio();

    std::shared_ptr<Graph::AudioGraph> getAudioGraph() {
        return audioGraph;
    }

    void set_output_block(size_t node_index, size_t block_output_index);

    ma_device get_audio_device() {
        return device;
    }

private:
    static void audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

    double clock_seconds;

    ma_device device;
    ma_device_config deviceConfig;
    std::shared_ptr<Graph::AudioGraph> audioGraph;
    size_t outputNodeIndex;
    size_t outputChannelStart;
};

#endif // AUDIO_ENGINE_H
