//
//
#include <src/core/graph.h>
#include <src/core/block.h>
#include <src/core/graph_io.h>
#include <numbers>

#ifndef RELEASE_SINEOSC_H
#define RELEASE_SINEOSC_H

class Oscillator : virtual public Graph::Block {
};

class SineOsc : public Oscillator, public InputOutput<2, 1> {
public:
    enum Inputs {
        FREQ,
        AMP
    };
    enum Outputs {
        OUT
    };

    explicit SineOsc(float freq = 110.0f, float amplitude = 1.0f) : phase(0.0f) {
        _inputs[FREQ] = freq;
        _inputs[AMP] = amplitude;
    }


    void process(Graph::AudioContext ctx) override {
        float freq = _inputs[FREQ];
        float amplitude = _inputs[AMP];
        float phase_inc = freq / ctx.sample_freq;
        _outputs[OUT] = amplitude * sinf(2.0f * std::numbers::pi * phase);

        phase = fmodf(phase + phase_inc, 1.0f);
    }

    std::string name() const override {
        return "SineOsc_" + std::to_string(_id);
    }

private:
    float phase;

};


#endif //RELEASE_SINEOSC_H
