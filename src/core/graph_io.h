
#ifndef RELEASE_GRAPH_IO_H
#define RELEASE_GRAPH_IO_H

#include "graph_deprecated.h"
#include "wire_deprecated.h"
#include "utils/assert.h"

template<size_t I, size_t O>
struct InputOutput {
    float inputs[I];
    float outputs[O];
    deprecated_Graph::Wire inputs_wires[I];

    InputOutput() {
        for (float &input: inputs) {
            input = 0.0f;
        }
        for (float &output: outputs) {
            output = 0.0f;
        }
        for (deprecated_Graph::Wire &wire: inputs_wires) {
            wire = deprecated_Graph::Wire();
        }
    }

    size_t
    connect(deprecated_Graph::Block *in, deprecated_Graph::Block *out, size_t in_index, size_t width, size_t out_index,
            float gain = 1.0f,
            float offset = 0.0f,
            deprecated_Graph::WireTransform transform = deprecated_Graph::WireTransform::NONE,
            float wire_transform_param = 0.0f) {
        ASSERT(in != nullptr);

        if (!(transform == deprecated_Graph::WireTransform::STEREO_PAN ||
              transform == deprecated_Graph::WireTransform::EXPAND)) {
            ASSERT(in_index + width <= in->output_size());
        }
        ASSERT(out_index + width <= I);

        int free_idx = -1;
        for (size_t i = 0; i < I; ++i) {
            const deprecated_Graph::Wire &wire = inputs_wires[i];
            if (wire.in == nullptr && free_idx == -1) {
                free_idx = (int) i;
            }
            if (out_index < wire.out_index + wire.width)
                throw std::runtime_error("Wires crossing");
        }
        if (free_idx == -1)
            throw std::runtime_error("No free wire");
        inputs_wires[free_idx] = deprecated_Graph::Wire(in, out, in_index, width, out_index, gain, offset, transform,
                                                        wire_transform_param);
        return inputs_wires[free_idx].id;
    }

    void disconnect(size_t out_index_or_id, bool is_id) {
        for (size_t i = 0; i < I; ++i) {
            deprecated_Graph::Wire &wire = inputs_wires[i];
            if ((is_id && wire.id == out_index_or_id) || (!is_id && wire.out_index == out_index_or_id)) {
                wire.in = nullptr;
                wire.in_index = 0;
                wire.width = 0;
                wire.out_index = 0;
                wire.id = 0;
                return;
            }
        }

        throw std::runtime_error("No matching wire found to disconnect_wire.");
    }

    void clear_wires() {
        for (size_t i = 0; i < I; ++i) {
            inputs_wires[i] = deprecated_Graph::Wire();
        }
    }

};

#define IMPLEMENT_BLOCK_IO(I, O) \
                                    \
    InputOutput<I, O> io;         \
                                    \
    virtual size_t input_size() const override { \
        return I; \
    } \
    \
    virtual size_t output_size() const override { \
        return O; \
    } \
    \
    virtual float *inputs() override { \
        return io.inputs; \
    } \
    \
    virtual const float *outputs() const override { \
        return io.outputs; \
    } \
    \
    virtual const deprecated_Graph::Wire * get_input_wires(size_t &size) const override { \
        size = I; \
        return io.inputs_wires; \
    } \
    \
    virtual size_t connect_wire(Block *in, size_t in_index, size_t width, size_t out_index, \
        float gain=1.0, float offset=0.0, deprecated_Graph::WireTransform transform = deprecated_Graph::WireTransform::NONE, float wire_transform_param=0.0) override { \
        return io.connect(in, this, in_index, width, out_index, gain, offset, transform, wire_transform_param); \
    } \
    \
    virtual void disconnect_wire(size_t out_index_or_id, bool is_id) override { \
        io.disconnect(out_index_or_id, is_id); \
    }                               \
    void clear_wires() {\
        io.clear_wires();\
    }\
    static size_t static_input_size() { \
        return I; \
    } \
    static size_t static_output_size() { \
        return O; \
    } \

#endif //RELEASE_GRAPH_IO_H
