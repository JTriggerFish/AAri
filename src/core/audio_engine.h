
#ifndef AUDIO_ENGINE_H
#define AUDIO_ENGINE_H


#include "../miniaudio.h"
#include <mutex>
#include "graph.h"
#include "graph_registry.h"
#include <memory>
#include <tuple>
#include <optional>
#include <tuple>
#include <entt/entt.hpp>

namespace AAri {
    class AudioEngine : public IGraphRegistry {
    public:
        AudioEngine(ma_uint32 sample_rate = 48000, ma_uint32 buffer_size = 512);

        ~AudioEngine();

        void startAudio();

        void stopAudio();


        /**
         * Any access to the registry is most likely not thread-safe with the callback
         * so we need to lock it
         * Outside of this class, this in particular used for block creation
         * @return a tuple containing the registry and a SpinLockGuard
         *
         */
        std::tuple<entt::registry &, SpinLockGuard> get_graph_registry() override {
            return {_graph.registry, SpinLockGuard(_callback_lock)};
        }

        ma_device get_audio_device() {
            return _device;
        }

        SpinLockGuard lock_till_function_returns() {
            return {_callback_lock};
        }

        Graph& _test_only_get_graph() {
            return _graph;
        }

        //Graph modification functions ----------------------------------------------
        void set_output_ref(entt::entity output_id, size_t output_width);

        entt::entity add_wire(entt::entity from_block,
                              entt::entity to_block,
                              entt::entity from_output,
                              entt::entity to_input,
                              TransmitFunc transmitFunc,
                              float gain = 1.0f, float offset = 0.0f);

        entt::entity add_wire_to_mixer(entt::entity from_block,
                                       entt::entity to_block,
                                       entt::entity from_output,
                                       size_t to_mixer_input_index,
                                       TransmitFunc transmitFunc,
                                       float gain = 1.0f, float offset = 0.0f);

        void remove_wire(entt::entity wire_id);

        void remove_block(entt::entity block_id);

        void tweak_wire_gain(entt::entity wire_id, float gain);

        void tweak_wire_offset(entt::entity wire_id, float offset);

        IoMap view_block_io(entt::entity block_id);

        //"free" inspection functions ----------------------------------------------
        // these can be called without locking the registry
        Block view_block(entt::entity block_id) const;

        Wire view_wire(entt::entity wire_id) const;


        std::vector<entt::entity> get_wires_to_block(entt::entity block_id) const;

        std::vector<entt::entity> get_wires_from_block(entt::entity block_id) const;

        std::optional<entt::entity> get_wire_to_input(entt::entity input_id) const;

        std::vector<entt::entity> get_wires_from_output(entt::entity output_id) const;

        std::vector<Block> get_blocks() const;

        std::tuple<entt::entity, size_t> get_output_ref() const;

        void set_input_1d(entt::entity input_id, float value);

        template<size_t N>
        void set_input_Nd(entt::entity input_id, const std::array<float, N>&value);

    private:
        static void audio_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);

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
