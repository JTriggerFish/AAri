//
//
#ifndef RELEASE_SINEOSC_H
#define RELEASE_SINEOSC_H

#include "../core/graph.h"
#include "../core/audio_context.h"
#include <entt/entt.hpp>
#include <cmath>


constexpr float PI = 3.1415927f;

namespace AAri {


    struct Oscillator {

    };

    struct SineOsc : public Oscillator {
        static void process(entt::registry &registry, const Block &block, AudioContext ctx);

        static entt::entity create(entt::registry &registry, float init_freq = 440.0f, float init_amp = 1.0f);
    };
}


#endif //RELEASE_SINEOSC_H
