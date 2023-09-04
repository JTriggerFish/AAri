//
//
#include <src/core/graph.h>
#include <src/core/block.h>
#include <src/core/graph_io.h>
#include <cmath>

#ifndef RELEASE_MIXERS_H
#define RELEASE_MIXERS_H

class Mixer : virtual public Graph::Block {
};

class MonoToStereo : public Mixer, public InputOutput<3, 2> {
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
        _inputs[AMP_DB] = amp_db;
        _inputs[PANNING] = panning;
    }


    void process(Graph::AudioContext ctx) override {
        const float amp_db = _inputs[AMP_DB];
        // Clamp panning to [-1, 1]
        const float panning = fminf(fmaxf(_inputs[PANNING], -1.0f), 1.0f);
        const float amp = powf(10.0f, amp_db / 20.0f);
        const float left_amp = amp * (1.0f - panning);
        const float right_amp = amp * panning;
        _outputs[LEFT] = _inputs[MONO_IN] * left_amp;
        _outputs[RIGHT] = _inputs[MONO_IN] * right_amp;
    }

    std::string name() const override {
        return "MonoToStereo_" + std::to_string(_id);
    }

};

class Affine : public Mixer, public InputOutput<2, 1> {
    /** Affine mixer
     *  Takes 1 input and 2 parameters
     *  Outputs the affine combinat_
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
        _inputs[A] = a;
        _inputs[B] = b;
    }


    void process(Graph::AudioContext ctx) override {
        const float a = _inputs[A];
        const float b = _inputs[B];
        _outputs[OUT] = a + b * _outputs[OUT];
    }

    std::string name() const override {
        return "Affine_" + std::to_string(id());
    }

};

class StereoMixer : public Mixer, public InputOutput<64, 2> {
public:

    enum Outputs {
        LEFT, RIGHT
    };

    StereoMixer() {
        for (float &input: _inputs) {
            input = 0.0f;
        }
    };

    void process(Graph::AudioContext ctx) override {
        double left = 0.0;
        double right = 0.0;
        for (int i = 0; i < 32; i += 2) {
            left += _inputs[i];
            right += _inputs[i + 1];
        }
        _outputs[LEFT] = float(left);
        _outputs[RIGHT] = float(right);
    }

    std::string name() const override {
        return "StereoMixer_" + std::to_string(_id);
    }


};

#endif //RELEASE_MIXERS_H
