//
//

#ifndef AARI_WIRES_H
#define AARI_WIRES_H

#include "blocks.h"
#include "inputs_outputs.h"
#include <string>
#include <vector>
#include <entt/entt.hpp>

namespace AAri {
    struct Wire;

    using TransmitFunc = std::function<void(entt::registry&, const Wire&)>;

    struct Wire {
        friend class AudioEngine;

        // Members ----------------------------------------------------------------------
        entt::entity from_block = entt::null;
        entt::entity to_block = entt::null;
        entt::entity from_output = entt::null;
        entt::entity to_input = entt::null;
        float gain = 1.0f;
        float offset = 0.0f;
        TransmitFunc transmitFunc = nullptr;
        //-------------------------------------------------------------------------------

        static void transmit_1d_to_1d(entt::registry&registry, const Wire&wire);

        template<size_t N>
        static void broadcast_1d_to_Nd(entt::registry&registry, const Wire&wire);

        //Templated function for mono mixers:
        template<size_t N>
        static void transmit_to_mono_mixer(entt::registry&registry, const Wire&wire);

        //Similar for stereo mixers:
        template<size_t N>
        static void transmit_stereo_to_stereo_mixer(entt::registry&registry, const Wire&wire);

        template<size_t N>
        static void transmit_mono_to_stereo_mixer(entt::registry&registry, const Wire&wire);

    private:
        // Creation and deletion are private
        // because they require a new topological sort of the graph
        // therefore should be done through the AudioEngine class
        static entt::entity create(entt::registry&registry,
                                   entt::entity from_block,
                                   entt::entity to_block,
                                   entt::entity from_output,
                                   entt::entity to_input,
                                   TransmitFunc transmitFunc,
                                   float gain = 1.0f, float offset = 0.0f) {
            //First check there isn't already a wire to this same input
            auto view = registry.view<Wire>();
            for (auto entity: view) {
                auto&wire = view.get<Wire>(entity);
                if (wire.to_input == to_input) {
                    throw std::runtime_error("Cannot create wire, input already connected");
                }
            }


            auto entity = registry.create();
            registry.emplace<Wire>(entity, from_block, to_block, from_output,
                                   to_input, gain, offset, transmitFunc);
            return entity;
        }

        static void destroy(entt::registry&registry, entt::entity entity) {
            registry.destroy(entity);
        }
    };
}
#endif //AARI_WIRES_H
