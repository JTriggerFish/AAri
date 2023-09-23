
#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H


#include "../miniaudio.h"
#include <mutex>
#include "graph_deprecated.h"
#include <memory>

class SpinLockGuard {
public:
    SpinLockGuard(ma_spinlock &spinlock) : spinlock(spinlock) {
        ma_spinlock_lock(&spinlock);
    }

    ~SpinLockGuard() {
        ma_spinlock_unlock(&spinlock);
    }

private:
    ma_spinlock &spinlock;
};

class AudioEngine {
public:
    AudioEngine(ma_uint32 sample_rate = 48000, ma_uint32 buffer_size = 512);

    ~AudioEngine();

    void startAudio();

    void stopAudio();

    std::shared_ptr<deprecated_Graph::AudioGraph> getAudioGraph() {
        return _audioGraph;
    }

    void set_output_block(size_t node_index, size_t block_output_index);

    ma_device get_audio_device() {
        return _device;
    }

    SpinLockGuard lock_till_function_returns() {
        return SpinLockGuard(_callback_lock);
    }

private:
    static void audio_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);

    double clock_seconds;

    ma_device _device;
    ma_device_config _deviceConfig;
    ma_spinlock _callback_lock = 0;

    std::shared_ptr<deprecated_Graph::AudioGraph> _audioGraph;
    size_t _outputNodeIndex;
    size_t _outputChannelStart;
};

#endif // AUDIO_ENGINE_H
