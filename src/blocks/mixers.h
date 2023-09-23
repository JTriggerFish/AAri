//
//
#include "../core/graph_deprecated.h"
#include "../core/graph_io.h"
#include <cmath>

#ifndef RELEASE_MIXERS_H
#define RELEASE_MIXERS_H

class Mixer : public deprecated_Graph::Block {
};

class MonoMixer : public Mixer {
public:
    IMPLEMENT_BLOCK_IO(32, 1);
    enum Outputs {
        OUT
    };

    MonoMixer() {
        for (float &input: io.inputs) {
            input = 0.0f;
        }
    };

    void process(deprecated_Graph::AudioContext ctx) override {
        double sum = 0.0;
        for (float input: io.inputs) {
            sum += input;
        }
        io.outputs[0] = float(sum);
    }

    std::string name() const override {
        return "MonoMixer_" + std::to_string(id());
    }
};


class StereoMixer : public Mixer {
public:
    IMPLEMENT_BLOCK_IO(64, 2);

    enum Outputs {
        LEFT, RIGHT
    };

    StereoMixer() {
        for (float &input: io.inputs) {
            input = 0.0f;
        }
    };

    void process(deprecated_Graph::AudioContext ctx) override {
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
