#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>
#include <algorithm>
#include <stdexcept>
#include <optional>

#define ASSERT(condition) \
    if (!(condition)) { \
        throw std::runtime_error("Assertion failed: " #condition); \
    }

namespace Graph {
    struct AudioContext {
        float sample_freq;
        double clock; // Current elapsed time in seconds
    };

    class Block;

    struct Wire {
        Block *in;
        Block *out;
        size_t in_index;
        size_t width;
        size_t out_index;
        size_t id;

        Wire(Block *in, Block *out,
             size_t in_index, size_t width,
             size_t out_index) : in(in), out(out), in_index(in_index), width(width), out_index(out_index) {
            id = ++_latest_id;
        }

        Wire() : in(nullptr), out(nullptr), in_index(0), width(0), out_index(0), id(0) {}


        Wire(const Wire &) = default;               // Copy constructor
        Wire &operator=(const Wire &) = default;

    private:
        static size_t _latest_id;
    };

    class Block {
    public:
        friend class AudioGraph;

        double last_processed_time;

        virtual size_t input_size() const = 0;

        virtual size_t output_size() const = 0;

        virtual float *inputs() const = 0;

        virtual float *outputs() const = 0;

        virtual Wire *get_input_wires(size_t &size) const = 0;

        virtual void process(AudioContext) = 0;


        virtual std::string name() = 0;

        size_t id() const { return _id; }

        virtual ~Block() = default;

    protected:
        size_t _id;

        Block() : last_processed_time(0.0) { _id = ++_latest_id; }

        virtual void connect_wire(Block *in, size_t in_index, size_t width, size_t out_index) = 0;

        virtual void disconnect_wire(size_t out_index_or_id, bool is_id) = 0;

    private:
        static size_t _latest_id;
    };

    template<size_t IN, size_t OUT>
    struct InputOutput {
        float inputs[IN] = {0};
        float outputs[OUT] = {0};
        Wire inputs_wires[IN];

        void connect(Block *in, Block *out, size_t in_index, size_t width, size_t out_index) {
            ASSERT(in != nullptr);
            ASSERT(in_index + width <= in->output_size());
            ASSERT(out_index + width <= IN);

            int free_idx = -1;
            for (size_t i = 0; i < IN; ++i) {
                const Wire &wire = inputs_wires[i];
                if (wire.in == nullptr)
                    free_idx = (int) i;
                if (out_index < wire.out_index + wire.width)
                    throw std::runtime_error("Wires crossing");
            }
            if (free_idx == -1)
                throw std::runtime_error("No free wire");
            inputs_wires[free_idx] = Wire(in, out, in_index, width, out_index);
        }

        void disconnect(size_t out_index_or_id, bool is_id) {
            for (size_t i = 0; i < IN; ++i) {
                Wire &wire = inputs_wires[i];
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


    class AudioGraph {
    public:
        void add_block(const std::shared_ptr<Block> &block);

        void remove_block(size_t block_id);

        void connect_wire(size_t in_block_id, size_t out_block_id, size_t in_index, size_t width, size_t out_index);

        void disconnect_wire(size_t wire_id, std::optional<size_t> out_block_id);

        void process(AudioContext ctx);

        AudioGraph();


    private:
        std::unordered_map<size_t, std::shared_ptr<Block> > _blocks;
        std::vector<Block *> _topologicalOrder;
        bool _ordered = false;

        void dfs(Block *vertex);

        void update_ordering();

        Block *find_wire_owner(size_t wire_id);

    private:
        // Temp memory
        std::unordered_map<Block *, bool> _visited;
        std::vector<Block *> _tempStack;
        std::unordered_multimap<Block *, Wire> _outgoingWires;

    };


}

#endif //GRAPH_H
