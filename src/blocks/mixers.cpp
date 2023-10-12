//
//

#include "mixers.h"

using namespace AAri;

template<int N>
void MonoMixer<N>::process(entt::registry &registry, const Block &block, AudioContext ctx) {
    auto &input = registry.get<InputND<N> >(block.inputIds[0]);
    auto &out = registry.get<Output1D>(block.outputIds[0]);

    out.value = 0.0f;
    for (int i = 0; i < N; i++) {
        out.value += input.value[i];
    }

}

template<int N>
entt::entity MonoMixer<N>::create(entt::registry &registry) {
    auto input = registry.create();
    registry.emplace<InputND<N>>(input, std::array<float, N>{0.0f}, ParamName::Input);
    auto output = registry.create();
    registry.emplace<Output1D>(output, 0.0f, ParamName::Out);

    return Block::create(registry, BlockType::MonoMixer,
                         std::array<entt::entity, 8>{input, entt::null, entt::null, entt::null, entt::null, entt::null,
                                                     entt::null, entt::null},
                         std::array<entt::entity, 4>{output, entt::null, entt::null, entt::null},
                         process);

}


template<int N>
void StereoMixer<N>::process(entt::registry &registry, const Block &block, AudioContext ctx) {
    auto &input = registry.get<InputNDStereo<N> >(block.inputIds[0]);
    auto &out = registry.get<Output2D>(block.outputIds[0]);

    out.value[0] = 0.0f;
    out.value[1] = 0.0f;
    for (int i = 0; i < N; i++) {
        out.value[0] += input.left[i];
        out.value[1] += input.right[i];
    }

}

template<int N>
entt::entity StereoMixer<N>::create(entt::registry &registry) {
    auto input = registry.create();
    registry.emplace<InputNDStereo<N> >(input, std::array<float, N>{0.0f}, std::array<float, N>{0.0f},
                                        ParamName::Input);
    auto output = registry.create();
    registry.emplace<Output2D>(output, std::array<float, 2>{0.0f, 0.0f}, ParamName::StereoOut);

    return Block::create(registry, BlockType::StereoMixer,
                         std::array<entt::entity, 8>{input, entt::null, entt::null, entt::null, entt::null, entt::null,
                                                     entt::null, entt::null},
                         std::array<entt::entity, 4>{output, entt::null, entt::null, entt::null},
                         process);
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


