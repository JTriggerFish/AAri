//
//
#include <src/core/graph.h>
#include <src/core/graph_io.h>
#include <cmath>

#ifndef RELEASE_MIXERS_H
#define RELEASE_MIXERS_H

class Mixer : public Graph::Block {
};

class MonoToStereo : public Mixer {
public:
    enum Inputs {
        MONO_IN,
        AMP_DB,
        PANNING,
    };
    enum Outputs {
        LEFT, RIGHT
    };

    MonoToStereo(float amp_db = -30.0f, float panning = 0.0f) {
        io.inputs[AMP_DB] = amp_db;
        io.inputs[PANNING] = panning;
    }

    IMPLEMENT_BLOCK_IO(3, 2);

    virtual void process(Graph::AudioContext ctx) override {
        const float amp_db = io.inputs[AMP_DB];
        // Clamp panning to [-1, 1]
        const float panning = fminf(fmaxf(io.inputs[PANNING], -1.0f), 1.0f);
        const float amp = powf(10.0f, amp_db / 20.0f);
        const float left_amp = amp * (1.0f - panning);
        const float right_amp = amp * panning;
        io.outputs[LEFT] = io.inputs[MONO_IN] * left_amp;
        io.outputs[RIGHT] = io.inputs[MONO_IN] * right_amp;
    }

    virtual std::string name() const override {
        return "MonoToStereo_" + std::to_string(id());
    }

};

class MonoMixer : public Mixer {

};

#endif //RELEASE_MIXERS_H
