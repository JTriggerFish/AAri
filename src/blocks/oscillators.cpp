//
//

#include "oscillators.h"
#include <array>

using namespace AAri;

void Phase::process(entt::registry &registry, entt::entity entity, AudioContext ctx) {
    auto phase = registry.get<Input1D>(entity);
    auto clock = registry.get<ClockLink>(entity);
    phase.value = fmodf(phase.value + clock.speed_multiplier * ctx.dt, 1.0f);
}

entt::entity Phase::create(entt::registry &registry, float sample_freq, float init_phase) {
    auto entity = registry.create();
    registry.emplace<Input1D>(entity, init_phase - 1.0f / sample_freq, ParamName::Phase);
    registry.emplace<ClockLink>(entity);
    return entity;
}

void SineOsc::process(entt::registry &registry, entt::entity entity, AudioContext ctx) {
    auto block = registry.get<Block>(entity);
    //Presumably here we don't need to check that this is a SineOsc
    auto &phase = registry.get<Input1D>(block.inputIds[0]);
    auto &freq = registry.get<Input1D>(block.inputIds[1]);
    auto &amp = registry.get<Input1D>(block.inputIds[2]);

    auto &out = registry.get<Output1D>(block.outputIds[0]);

    out.value = amp.value * sinf(2.0f * PI * freq.value * phase.value);
}

entt::entity
SineOsc::create(entt::registry &registry, float sample_freq, float init_phase, float init_freq, float init_amp) {
    auto entity = registry.create();
    auto phase = Phase::create(registry, sample_freq, init_phase);
    auto freq = registry.create();
    registry.emplace<Input1D>(freq, init_freq, ParamName::Freq);
    auto amp = registry.create();
    registry.emplace<Input1D>(amp, init_amp, ParamName::Amp);
    auto out = registry.create();
    registry.emplace<Output1D>(out, 0.0f, ParamName::Out);

    auto block = registry.create();
    registry.emplace<Block>(block, std::array<entt::entity, 8>{phase, freq, amp, entt::null, entt::null, entt::null,
                                                               entt::null, entt::null},
                            std::array<entt::entity, 4>{out, entt::null, entt::null, entt::null}, BlockType::SineOsc);
    return entity;
}
