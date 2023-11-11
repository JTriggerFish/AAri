//
//

#ifndef AARI_INPUTS_OUTPUTS_H
#define AARI_INPUTS_OUTPUTS_H

#include <cstddef>
#include <array>


namespace AAri {
// Paramaters
    enum class ParamName {
        NONE,
        Input,
        Freq,
        Phase,
        Amp,
        Attack,
        Decay,
        Sustain,
        Release,

        Out,
        StereoOut,
    };

    struct InputOutput {
        // This is mainly to make the class polymorphic
        // so that pybind11 can recognize it as such
        // and we can have polymorphic containers
        virtual ~InputOutput() = default;
    };

    struct Input1D : public InputOutput {
        float value = 0.0;
        ParamName name = ParamName::NONE;

        Input1D(float value, ParamName name) : value(value), name(name) {};
    };

    struct Input2D : public InputOutput {
        std::array<float, 2> value = {0.0, 0.0};
        ParamName name = ParamName::NONE;

        Input2D(std::array<float, 2> value, ParamName name) : value(value), name(name) {};
    };

    struct Output1D : public InputOutput {
        float value = 0.0;
        ParamName name = ParamName::NONE;

        Output1D(float value, ParamName name) : value(value), name(name) {};
    };

    struct Output2D : public InputOutput {
        std::array<float, 2> value = {0.0, 0.0};
        ParamName name = ParamName::NONE;

        Output2D(std::array<float, 2> value, ParamName name) : value(value), name(name) {};
    };

    template<size_t N>
    struct InputND : public InputOutput {
        std::array<float, N> value = {0.0};
        ParamName name = ParamName::NONE;

        InputND(std::array<float, N> value, ParamName name) : value(value), name(name) {};
    };

    template<size_t N>
    struct InputNDStereo : public InputOutput {
        std::array<float, N> left = {0.0};
        std::array<float, N> right = {0.0};
        ParamName name = ParamName::NONE;

        InputNDStereo(std::array<float, N> left, std::array<float, N> right, ParamName name) : left(left), right(right),
                                                                                               name(name) {};
    };

    template<size_t N>
    struct OutputND : public InputOutput {
        std::array<float, N> value = {0.0};
        ParamName name = ParamName::NONE;

        OutputND(std::array<float, N> value, ParamName name) : value(value), name(name) {};
    };
}
#endif //AARI_INPUTS_OUTPUTS_H
