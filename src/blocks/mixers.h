//
//

#ifndef RELEASE_MIXERS_H
#define RELEASE_MIXERS_H

#include "../core/graph.h"
#include "../core/audio_context.h"
#include "../core/graph_registry.h"
#include <entt/entt.hpp>
#include <cmath>

namespace AAri {
    struct Mixer {
        //Note that in mixers we abuse the system a bit by using entt::entity types
        // to store the index of the inputs in an array of floats

    };

    //A mixer with a templated input size
    template<size_t N>
    struct MonoMixer : public Mixer {
        static void process(entt::registry &registry, const Block &block, AudioContext ctx);

        static entt::entity create(IGraphRegistry *reg);
    };


    template<size_t N>
    struct StereoMixer : public Mixer {
        static void process(entt::registry &registry, const Block &block, AudioContext ctx);

        static entt::entity create(IGraphRegistry *reg);
    };


}


#endif //RELEASE_MIXERS_H
