//
//

#ifndef AARI_AUDIO_CONTEXT_H
#define AARI_AUDIO_CONTEXT_H


struct AudioContext {
    float sample_freq;
    float dt;
    double clock; // Current elapsed time in seconds
};
#endif //AARI_AUDIO_CONTEXT_H
