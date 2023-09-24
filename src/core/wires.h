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
    struct Wire {
        entt::entity from = entt::null;
        entt::entity to = entt::null;
        float gain = 1.0f;
        float offset = 0.0f;
    };

    static entt::entity create(entt::registry &registry, entt::entity from, entt::entity to, float gain = 1.0f,
                               float offset = 0.0f) {
        auto entity = registry.create();
        registry.emplace<Wire>(entity, from, to, gain, offset);
        return entity;
    }

    static void destroy(entt::registry &registry, entt::entity entity) {
        registry.destroy(entity);
    }
}
#endif //AARI_WIRES_H
