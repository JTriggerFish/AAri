//
//
#include <core/graph.h>
#include <core/graph_io.h>
#include <cmath>

#ifndef RELEASE_ENVELOPES_H
#define RELEASE_ENVELOPES_H

#endif //RELEASE_ENVELOPES_H


class Product : public Graph::Block {
public:
    enum ProductType {
        DUAL_CHANNELS, // 2 independent channels doing one product each
        CASCADE// First output is the product of first two inputs, second output is the product of four outputs
    };
    enum Outputs {
        OUT1,
        OUT2
    };
    IMPLEMENT_BLOCK_IO(4, 2);

    Product(ProductType ptype) : _ptype(ptype) {
        for (float &input: io.inputs) {
            input = 1.0f;
        }
    };

    void process(Graph::AudioContext ctx) override {
        for (int i = 0; i < 2; i += 2) {
            io.outputs[i] = io.inputs[2 * i] * io.inputs[2 * i + 1];
        }
        if (_ptype == ProductType::CASCADE) {
            io.outputs[1] = io.outputs[0] * io.outputs[1];
        }
    }

    std::string name() const override {
        return "Product_" + std::to_string(id());
    }

protected:
    ProductType _ptype;
};