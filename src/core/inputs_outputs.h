//
//

#ifndef AARI_INPUTS_OUTPUTS_H
#define AARI_INPUTS_OUTPUTS_H

#include <cstddef>
#include <array>


namespace AAri {
    // Paramaters

    struct InputOutput {
        // This is mainly to make the class polymorphic
        // so that pybind11 can recognize it as such
        // and we can have polymorphic containers
        virtual ~InputOutput() = default;
    };

    struct Input1D : public InputOutput {
        float value = 0.0;

        Input1D(float value) : value(value) {
        };
    };

    struct Input2D : public InputOutput {
        std::array<float, 2> value = {0.0, 0.0};

        Input2D(std::array<float, 2> value): value(value) {
        };
    };

    struct Output1D : public InputOutput {
        float value = 0.0;

        Output1D(float value) : value(value) {
        };
    };

    struct Output2D : public InputOutput {
        std::array<float, 2> value = {0.0, 0.0};

        Output2D(std::array<float, 2> value): value(value) {
        };
    };

    template<size_t N>
    struct InputND : public InputOutput {
        std::array<float, N> value = {0.0};

        InputND(std::array<float, N> value) : value(value) {
        };
    };

    template<size_t N>
    struct InputNDStereo : public InputOutput {
        std::array<float, N> left = {0.0};
        std::array<float, N> right = {0.0};

        InputNDStereo(std::array<float, N> left, std::array<float, N> right): left(left), right(right) {
        };
    };

    template<size_t N>
    struct OutputND : public InputOutput {
        std::array<float, N> value = {0.0};

        OutputND(std::array<float, N> value): value(value) {
        };
    };
}
#endif //AARI_INPUTS_OUTPUTS_H
