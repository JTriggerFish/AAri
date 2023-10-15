//
//

#include "oscillators.h"

using namespace AAri;


void SineOsc::process(entt::registry &registry, const Block &block, AudioContext ctx) {
    auto &phase = registry.get<Input1D>(block.inputIds[0]);
    auto &freq = registry.get<Input1D>(block.inputIds[1]);
    auto &amp = registry.get<Input1D>(block.inputIds[2]);
    auto &out = registry.get<Output1D>(block.outputIds[0]);

    out.value = amp.value * sinf(2.0f * PI * freq.value * phase.value);
    phase.value = fmodf(phase.value + ctx.dt, 1.0f);
}

entt::entity
SineOsc::create(entt::registry &registry, float init_freq, float init_amp) {
    auto phase = registry.create();
    registry.emplace<Input1D>(phase, 0.0f, ParamName::Phase);
    auto freq = registry.create();
    registry.emplace<Input1D>(freq, init_freq, ParamName::Freq);
    auto amp = registry.create();
    registry.emplace<Input1D>(amp, init_amp, ParamName::Amp);
    auto out = registry.create();
    registry.emplace<Output1D>(out, 0.0f, ParamName::Out);

    return Block::create(registry, BlockType::SineOsc,
                         fill_with_null<N_INPUTS>(phase, freq, amp),
                         fill_with_null<N_OUTPUTS>(out),
                         process);
}
