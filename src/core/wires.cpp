//
//
#include "wires.h"

void AAri::Wire::transmit_1d_to_1d(entt::registry &registry, const AAri::Wire &wire) {
    auto &from_output = registry.get<Input1D>(wire.from_output);
    auto &to_input = registry.get<Input1D>(wire.to_input);

    to_input.value = from_output.value * wire.gain + wire.offset;
}

void AAri::Wire::broadcast_1d_to_2d(entt::registry &registry, const AAri::Wire &wire) {
    auto &from_output = registry.get<Input1D>(wire.from_output);
    auto &to_input = registry.get<Input2D>(wire.to_input);

    const float value = from_output.value * wire.gain + wire.offset;
    to_input.value[0] = value;
    to_input.value[1] = value;
}
