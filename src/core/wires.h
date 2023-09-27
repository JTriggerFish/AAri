//
//

#ifndef AARI_WIRES_H
#define AARI_WIRES_H

#include "blocks.h"
#include "parameters.h"
#include <string>
#include <vector>
#include <entt/entt.hpp>

namespace AAri {
    struct Wire;

    typedef void (*TransmitFunc)(entt::registry &registry, const Wire &wire);

    struct Wire {
        // Members ----------------------------------------------------------------------
        entt::entity from_block = entt::null;
        entt::entity to_block = entt::null;
        entt::entity from_output = entt::null;
        entt::entity to_input = entt::null;
        float gain = 1.0f;
        float offset = 0.0f;
        TransmitFunc transmitFunc = nullptr;
        //-------------------------------------------------------------------------------

        static entt::entity create(entt::registry &registry,
                                   entt::entity from_block,
                                   entt::entity to_block,
                                   entt::entity from_output,
                                   entt::entity to_input,
                                   TransmitFunc transmitFunc,
                                   float gain = 1.0f, float offset = 0.0f) {
            auto entity = registry.create();
            registry.emplace<Wire>(entity, from_block, to_block, from_output,
                                   to_input, gain, offset, transmitFunc);
            return entity;
        }

        static void destroy(entt::registry &registry, entt::entity entity) {
            registry.destroy(entity);
        }

        static void transmit_1d_to_1d(entt::registry &registry, const Wire &wire);

        static void broadcast_1d_to_2d(entt::registry &registry, const Wire &wire);
    };
}
#endif //AARI_WIRES_H
