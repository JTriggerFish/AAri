//
//

#ifndef AARI_GRAPH_H
#define AARI_GRAPH_H

#include <entt/entt.hpp>
#include <vector>
#include <string>
#include "parameters.h"
#include "blocks.h"

namespace Graph {
    struct AudioContext {
        float sample_freq;
        float dt;
        double clock; // Current elapsed time in seconds
    };
    struct ClockLink {
        float speed_multiplier = 1.0;
    };

    //Wires
    struct Wire {
        entt::entity from = entt::null;
        entt::entity to = entt::null;
        float gain = 1.0f;
        float offset = 0.0f;
    };


    //Specific blocks:
    struct Phase {
        static void process(entt::registry &registry, entt::entity entity, AudioContext ctx) {
            auto phase = registry.get<Input1D>(entity);
            auto clock = registry.get<ClockLink>(entity);
            phase.value = fmodf(phase.value + clock.speed_multiplier * ctx.dt, 1.0f);
        }

        static entt::entity create(entt::registry &registry, float sample_freq, float init_phase = 0.0f) {
            auto entity = registry.create();
            registry.emplace<Input1D>(entity, init_phase - 1.0f / sample_freq, ParamName::Phase);
            registry.emplace<ClockLink>(entity);
            return entity;
        }
    };

    struct SineOsc {
        static void process(entt::registry &registry, entt::entity entity, AudioContext ctx) {
            constexpr float pi = 3.14159265358979323846f;
            auto block = registry.get<Block>(entity);
            //Presumably here we don't need to check that this is a SineOsc
            auto &phase = registry.get<Input1D>(block.inputIds[0]);
            auto &freq = registry.get<Input1D>(block.inputIds[1]);
            auto &amp = registry.get<Input1D>(block.inputIds[2]);

            auto &out = registry.get<Output1D>(block.outputIds[0]);

            out.value = amp.value * sinf(2.0f * pi * freq.value * phase.value);
        }

        static entt::entity create(entt::registry &registry, float sample_freq, float init_phase = 0.0f,
                                   float init_freq = 440.0f, float init_amp = 1.0f) {
            auto entity = registry.create();
            auto phase = Phase::create(registry, sample_freq, init_phase);
            auto freq = registry.create();
            registry.emplace<Input1D>(freq, init_freq, ParamName::Freq);
            auto amp = registry.create();
            registry.emplace<Input1D>(amp, init_amp, ParamName::Amp);
            auto out = registry.create();
            registry.emplace<Output1D>(out, 0.0f, ParamName::Out);

            auto block = registry.create();
            registry.emplace<Block>(block, std::vector<entt::entity>{phase, freq, amp},
                                    std::vector<entt::entity>{out}, BlockType::SineOsc);
            return entity;
        }
    };

    // Setting up the registry
    entt::registry graph_registry;
}


#endif //AARI_GRAPH_H
