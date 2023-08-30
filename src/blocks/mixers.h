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

    explicit MonoToStereo(float amp_db = -30.0f, float panning = 0.0f) {
        io.inputs[AMP_DB] = amp_db;
        io.inputs[PANNING] = panning;
    }

    IMPLEMENT_BLOCK_IO(3, 2);

    void process(Graph::AudioContext ctx) override {
        const float amp_db = io.inputs[AMP_DB];
        // Clamp panning to [-1, 1]
        const float panning = fminf(fmaxf(io.inputs[PANNING], -1.0f), 1.0f);
        const float amp = powf(10.0f, amp_db / 20.0f);
        const float left_amp = amp * (1.0f - panning);
        const float right_amp = amp * panning;
        io.outputs[LEFT] = io.inputs[MONO_IN] * left_amp;
        io.outputs[RIGHT] = io.inputs[MONO_IN] * right_amp;
    }

    std::string name() const override {
        return "MonoToStereo_" + std::to_string(id());
    }

};

class Affine : public Mixer {
    /** Affine mixer
     *  Takes 1 input and 2 parameters
     *  Outputs the affine combination
     *  y = a + b * y
     */
public:
    enum Inputs {
        A,
        B,
    };
    enum Outputs {
        OUT
    };

    explicit Affine(float a = 0.0f, float b = 1.0f) {
        io.inputs[A] = a;
        io.inputs[B] = b;
    }

    IMPLEMENT_BLOCK_IO(2, 1);

    void process(Graph::AudioContext ctx) override {
        const float a = io.inputs[A];
        const float b = io.inputs[B];
        io.outputs[OUT] = a + b * io.outputs[OUT];
    }

    std::string name() const override {
        return "Affine_" + std::to_string(id());
    }

};

class StereoMixer : public Mixer {
public:
    IMPLEMENT_BLOCK_IO(32, 2);

    enum Outputs {
        LEFT, RIGHT
    };

    StereoMixer() {
        for (float &input: io.inputs) {
            input = 0.0f;
        }
    };

    void process(Graph::AudioContext ctx) override {
        double left = 0.0;
        double right = 0.0;
        for (int i = 0; i < 32; i += 2) {
            left += io.inputs[i];
            right += io.inputs[i + 1];
        }
        io.outputs[LEFT] = float(left);
        io.outputs[RIGHT] = float(right);
    }

    std::string name() const override {
        return "StereoMixer_" + std::to_string(id());
    }


};

#endif //RELEASE_MIXERS_H
