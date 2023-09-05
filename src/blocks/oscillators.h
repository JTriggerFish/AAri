//
//
#include <src/core/graph.h>
#include <src/core/graph_io.h>
#include <cmath>


#ifndef RELEASE_SINEOSC_H
#define RELEASE_SINEOSC_H

constexpr float pi = 3.1415927f;

class Oscillator : public Graph::Block {
};

class SineOsc : public Oscillator {
public:
    enum Inputs {
        FREQ,
        AMP
    };
    enum Outputs {
        OUT
    };

    explicit SineOsc(float freq = 110.0f, float amplitude = 1.0f) : phase(0.0f) {
        io.inputs[FREQ] = freq;
        io.inputs[AMP] = amplitude;
    }

    IMPLEMENT_BLOCK_IO(2, 1);

    void process(Graph::AudioContext ctx) override {
        float freq = io.inputs[FREQ];
        float amplitude = io.inputs[AMP];
        float phase_inc = freq / ctx.sample_freq;
        io.outputs[OUT] = amplitude * sinf(2.0f * pi * phase);

        phase = fmodf(phase + phase_inc, 1.0f);
    }

    std::string name() const override {
        return "SineOsc_" + std::to_string(id());
    }

private:
    float phase;

};


#endif //RELEASE_SINEOSC_H
