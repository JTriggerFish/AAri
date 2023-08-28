
#include "audio_engine.h"
#include <xmmintrin.h>
#include <pmmintrin.h>

void disable_denormals() {
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
}

//TODO be able to specify the output block to use from the graph