//
//
#include "wires.h"
#include "../blocks/mixers.h"

void AAri::Wire::transmit_1d_to_1d(entt::registry&registry, const AAri::Wire&wire) {
    auto&from_output = registry.get<Output1D>(wire.from_output);
    auto&to_input = registry.get<Input1D>(wire.to_input);

    to_input.value = from_output.value * wire.gain + wire.offset;
}

template<size_t N>
void AAri::Wire::broadcast_1d_to_Nd(entt::registry&registry, const AAri::Wire&wire) {
    auto&from_output = registry.get<Output1D>(wire.from_output);
    auto&to_input = registry.get<InputND<N>>(wire.to_input);

    const float value = from_output.value * wire.gain + wire.offset;
    for (size_t i = 0; i < N; i++) {
        to_input.value[i] = value;
    }
}

template<size_t N>
void AAri::Wire::transmit_to_mono_mixer(entt::registry&registry, const AAri::Wire&wire) {
    //Note that in mixers we abuse the system a bit by using the wire input ids to store the index of the input
    //using an entt::entity type to represent a simple array index
    auto&from_output = registry.get<Output1D>(wire.from_output);
    auto&mixer = registry.get<Block>(wire.to_block);
    auto&to_input = registry.get<InputND<N>>(mixer.inputIds[0]);
    to_input.value[(size_t)wire.to_input] = from_output.value * wire.gain + wire.offset;
}

template<size_t N>
void AAri::Wire::transmit_mono_to_stereo_mixer(entt::registry&registry, const AAri::Wire&wire) {
    //Note that in mixers we abuse the system a bit by using the wire input ids to store the index of the input
    //using an entt::entity type to represent a simple array index
    auto&from_output = registry.get<Output1D>(wire.from_output);
    auto&mixer = registry.get<Block>(wire.to_block);
    auto&to_input = registry.get<InputNDStereo<N>>(mixer.inputIds[0]);
    to_input.left[(size_t)wire.to_input] = from_output.value * wire.gain + wire.offset;
    to_input.right[(size_t)wire.to_input] = from_output.value * wire.gain + wire.offset;
}

template<size_t N>
void AAri::Wire::transmit_stereo_to_stereo_mixer(entt::registry&registry, const AAri::Wire&wire) {
    //Note that in mixers we abuse the system a bit by using the wire input ids to store the index of the input
    //using an entt::entity type to represent a simple array index
    auto&from_output = registry.get<OutputND<2>>(wire.from_output);
    auto&mixer = registry.get<Block>(wire.to_block);
    auto&to_input = registry.get<InputNDStereo<N>>(mixer.inputIds[0]);
    to_input.left[(size_t)wire.to_input] = from_output.value[0] * wire.gain + wire.offset;
    to_input.right[(size_t)wire.to_input] = from_output.value[1] * wire.gain + wire.offset;
}

//Explicit template instantiation of transmit functions for powers of 2
template void AAri::Wire::broadcast_1d_to_Nd<2>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::broadcast_1d_to_Nd<4>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::broadcast_1d_to_Nd<8>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::broadcast_1d_to_Nd<16>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::broadcast_1d_to_Nd<32>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_to_mono_mixer<2>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_to_mono_mixer<4>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_to_mono_mixer<8>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_to_mono_mixer<16>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_to_mono_mixer<32>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_mono_to_stereo_mixer<2>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_mono_to_stereo_mixer<4>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_mono_to_stereo_mixer<8>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_mono_to_stereo_mixer<16>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_mono_to_stereo_mixer<32>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_stereo_to_stereo_mixer<2>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_stereo_to_stereo_mixer<4>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_stereo_to_stereo_mixer<8>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_stereo_to_stereo_mixer<16>(entt::registry&registry, const AAri::Wire&wire);

template void AAri::Wire::transmit_stereo_to_stereo_mixer<32>(entt::registry&registry, const AAri::Wire&wire);



