//
//

#ifndef AARI_PARAMETERS_H
#define AARI_PARAMETERS_H


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
    struct Input1D {
        float value = 0.0;
        ParamName name = ParamName::NONE;
    };
    struct Input2D {
        float value[2] = {0.0, 0.0};
        ParamName name = ParamName::NONE;
    };
    struct Output1D {
        float value = 0.0;
        ParamName name = ParamName::NONE;
    };
    struct Output2D {
        float value[2] = {0.0, 0.0};
        ParamName name = ParamName::NONE;
    };
}
#endif //AARI_PARAMETERS_H
