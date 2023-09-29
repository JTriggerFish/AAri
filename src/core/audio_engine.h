
#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H


#include "../miniaudio.h"
#include <mutex>
#include "graph.h"
#include <memory>
#include <tuple>

namespace AAri {
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

        void set_output(entt::entity output_id, size_t output_width);

        /**
         * Any access to the regitry is most likely not thread-safe with the callback
         * so we need to lock it
         * @param registry
         * @return
         */
        std::tuple<entt::registry &, SpinLockGuard> get_graph_registry() {
            return {_graph.registry, SpinLockGuard(_callback_lock)};
        }

        ma_device get_audio_device() {
            return _device;
        }

        SpinLockGuard lock_till_function_returns() {
            return {_callback_lock};
        }

        Graph &_test_only_get_graph() {
            return _graph;
        }

    private:
        static void audio_callback(ma_device *pDevice, void *pOutput, const void *pInput, ma_uint32 frameCount);

        double clock_seconds;

        ma_device _device;
        ma_device_config _deviceConfig;
        ma_spinlock _callback_lock = 0;

        Graph _graph;
        entt::entity _output_id;
        size_t _output_width;
    };
}

#endif // AUDIO_ENGINE_H
