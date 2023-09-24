//
//
#ifndef RELEASE_SINEOSC_H
#define RELEASE_SINEOSC_H

#include "../core/graph.h"
#include <entt/entt.hpp>
#include <cmath>


constexpr float PI = 3.1415927f;

namespace AAri {

    struct Phase {
        static void process(entt::registry &registry, entt::entity entity, AudioContext ctx);

        static entt::entity create(entt::registry &registry, float sample_freq, float init_phase = 0.0f);
    };

    struct SineOsc {
        static void process(entt::registry &registry, entt::entity entity, AudioContext ctx);

        static entt::entity create(entt::registry &registry, float sample_freq, float init_phase = 0.0f,
                                   float init_freq = 440.0f, float init_amp = 1.0f);
    };
}


#endif //RELEASE_SINEOSC_H
