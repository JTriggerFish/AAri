//
//

#include "mixers.h"

using namespace AAri;

template<size_t N>
void MonoMixer<N>::process(entt::registry&registry, const Block&block, AudioContext ctx) {
    auto&input = registry.get<InputND<N>>(block.inputIds[0]);
    auto&out = registry.get<Output1D>(block.outputIds[0]);

    out.value = 0.0f;
    for (size_t i = 0; i < N; i++) {
        out.value += input.value[i];
    }
}

template<size_t N>
entt::entity MonoMixer<N>::create(IGraphRegistry* reg) {
    auto [registry, guard] = reg->get_graph_registry();
    auto input = registry.create();
    registry.emplace<InputND<N>>(input, std::array<float, N>{0.0f});
    auto output = registry.create();
    registry.emplace<Output1D>(output, 0.0f);

    return Block::create(registry, BlockType::MonoMixer,
                         fill_with_null<N_INPUTS>(input),
                         fill_with_null<N_OUTPUTS>(output),
                         process, nullptr);
}


template<size_t N>
void StereoMixer<N>::process(entt::registry&registry, const Block&block, AudioContext ctx) {
    auto&input = registry.get<InputNDStereo<N>>(block.inputIds[0]);
    auto&out = registry.get<OutputND<2>>(block.outputIds[0]);

    out.value[0] = 0.0f;
    out.value[1] = 0.0f;
    for (size_t i = 0; i < N; i++) {
        out.value[0] += input.left[i];
        out.value[1] += input.right[i];
    }
}

template<size_t N>
entt::entity StereoMixer<N>::create(IGraphRegistry* reg) {
    auto [registry, guard] = reg->get_graph_registry();
    auto input = registry.create();
    registry.emplace<InputNDStereo<N>>(input, std::array<float, N>{0.0f}, std::array<float, N>{0.0f});
    auto output = registry.create();
    registry.emplace<OutputND<2>>(output, std::array<float, 2>{0.0f, 0.0f});

    return Block::create(registry, BlockType::StereoMixer,
                         fill_with_null<N_INPUTS>(input),
                         fill_with_null<N_OUTPUTS>(output),
                         process, nullptr);
}


//Explicit template instantiations of power of 2 mixers
template
struct AAri::MonoMixer<2>;
template
struct AAri::MonoMixer<4>;
template
struct AAri::MonoMixer<8>;
template
struct AAri::MonoMixer<16>;
template
struct AAri::MonoMixer<32>;

template
struct AAri::StereoMixer<2>;
template
struct AAri::StereoMixer<4>;
template
struct AAri::StereoMixer<8>;
template
struct AAri::StereoMixer<16>;
template
struct AAri::StereoMixer<32>;


