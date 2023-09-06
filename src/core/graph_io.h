
#ifndef RELEASE_GRAPH_IO_H
#define RELEASE_GRAPH_IO_H

#include "graph.h"
#include "wire.h"

template<size_t IN, size_t OUT>
struct InputOutput {
    float inputs[IN] = {0};
    float outputs[OUT] = {0};
    Graph::Wire inputs_wires[IN];

    size_t connect(Graph::Block *in, Graph::Block *out, size_t in_index, size_t width, size_t out_index,
                   float gain = 1.0f,
                   float offset = 0.0f,
                   Graph::WireTransform transform = Graph::WireTransform::NONE,
                   float wire_transform_param = 0.0f) {
        ASSERT(in != nullptr);

        if (!(transform == Graph::WireTransform::STEREO_PAN || transform == Graph::WireTransform::EXPAND)) {
            ASSERT(in_index + width <= in->output_size());
        }
        ASSERT(out_index + width <= IN);

        int free_idx = -1;
        for (size_t i = 0; i < IN; ++i) {
            const Graph::Wire &wire = inputs_wires[i];
            if (wire.in == nullptr && free_idx == -1) {
                free_idx = (int) i;
            }
            if (out_index < wire.out_index + wire.width)
                throw std::runtime_error("Wires crossing");
        }
        if (free_idx == -1)
            throw std::runtime_error("No free wire");
        inputs_wires[free_idx] = Graph::Wire(in, out, in_index, width, out_index, gain, offset, transform,
                                             wire_transform_param);
        return inputs_wires[free_idx].id;
    }

    void disconnect(size_t out_index_or_id, bool is_id) {
        for (size_t i = 0; i < IN; ++i) {
            Graph::Wire &wire = inputs_wires[i];
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

};

#define IMPLEMENT_BLOCK_IO(IN, OUT) \
                                    \
    InputOutput<IN, OUT> io;         \
                                    \
    virtual size_t input_size() const override { \
        return IN; \
    } \
    \
    virtual size_t output_size() const override { \
        return OUT; \
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
    virtual const Graph::Wire * get_input_wires(size_t &size) const override { \
        size = IN; \
        return io.inputs_wires; \
    } \
    \
    virtual size_t connect_wire(Block *in, size_t in_index, size_t width, size_t out_index, \
        float gain=1.0, float offset=0.0, Graph::WireTransform transform = Graph::WireTransform::NONE, float wire_transform_param=0.0) override { \
        return io.connect(in, this, in_index, width, out_index, gain, offset, transform, wire_transform_param); \
    } \
    \
    virtual void disconnect_wire(size_t out_index_or_id, bool is_id) override { \
        io.disconnect(out_index_or_id, is_id); \
    } \
    static size_t static_input_size() { \
        return IN; \
    } \
    static size_t static_output_size() { \
        return OUT; \
    } \

#endif //RELEASE_GRAPH_IO_H
