//
//

#ifndef AARI_AUDIO_CONTEXT_H
#define AARI_AUDIO_CONTEXT_H

#include "utils/data_structures.h"
#include "wires.h"
#include "blocks.h"
#include "parameters.h"
#include <stack>
#include <string>
#include <vector>

struct AudioContext {
    float sample_freq;
    float dt;
    double clock; // Current elapsed time in seconds
};
#endif //AARI_AUDIO_CONTEXT_H
