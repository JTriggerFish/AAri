//
//

#include "parameters.h"


//Explicit instantiation of power of 2 output and input structss sizes
template
struct AAri::InputND<2>;
template
struct AAri::InputND<4>;
template
struct AAri::InputND<8>;
template
struct AAri::InputND<16>;
template
struct AAri::InputND<32>;

template
struct AAri::InputNDStereo<2>;
template
struct AAri::InputNDStereo<4>;
template
struct AAri::InputNDStereo<8>;
template
struct AAri::InputNDStereo<16>;
template
struct AAri::InputNDStereo<32>;

template
struct AAri::OutputND<2>;
template
struct AAri::OutputND<4>;
template
struct AAri::OutputND<8>;
template
struct AAri::OutputND<16>;
template
struct AAri::OutputND<32>;
