//
//
#include "wires.h"

void AAri::Wire::transmit_1d_to_1d(entt::registry &registry, const AAri::Wire &wire) {
    auto &from_output = registry.get<Output1D>(wire.from_output);
    auto &to_input = registry.get<Input1D>(wire.to_input);

    to_input.value = from_output.value * wire.gain + wire.offset;
}

void AAri::Wire::broadcast_1d_to_2d(entt::registry &registry, const AAri::Wire &wire) {
    auto &from_output = registry.get<Output1D>(wire.from_output);
    auto &to_input = registry.get<Input2D>(wire.to_input);

    const float value = from_output.value * wire.gain + wire.offset;
    to_input.value[0] = value;
    to_input.value[1] = value;
}

template<size_t N>
void AAri::Wire::transmit_to_mono_mixer(entt::registry &registry, const AAri::Wire &wire) {
    //Note that in mixers we abuse the system a bit by using the wire input ids to store the index of the input
    //using an entt::entity type to represent a simple array index
    auto &from_output = registry.get<Output1D>(wire.from_output);
    auto &to_input = registry.get<InputND<N> >(wire.to_input);
    to_input.value[(size_t) wire.to_input] = from_output.value * wire.gain + wire.offset;
}

template<size_t N>
void AAri::Wire::transmit_mono_to_stereo_mixer(entt::registry &registry, const AAri::Wire &wire) {
    //Note that in mixers we abuse the system a bit by using the wire input ids to store the index of the input
    //using an entt::entity type to represent a simple array index
    auto &from_output = registry.get<Output1D>(wire.from_output);
    auto &to_input = registry.get<InputNDStereo<N> >(wire.to_input);
    to_input.left[(size_t) wire.to_input] = from_output.value * wire.gain + wire.offset;
    to_input.right[(size_t) wire.to_input] = from_output.value * wire.gain + wire.offset;
}

template<size_t N>
void AAri::Wire::transmit_stereo_to_stereo_mixer(entt::registry &registry, const AAri::Wire &wire) {
    //Note that in mixers we abuse the system a bit by using the wire input ids to store the index of the input
    //using an entt::entity type to represent a simple array index
    auto &from_output = registry.get<Output2D>(wire.from_output);
    auto &to_input = registry.get<InputNDStereo<N> >(wire.to_input);
    to_input.left[(size_t) wire.to_input] = from_output.value[0] * wire.gain + wire.offset;
    to_input.right[(size_t) wire.to_input] = from_output.value[1] * wire.gain + wire.offset;

}


