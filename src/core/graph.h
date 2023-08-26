#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>
#include <algorithm>

namespace Graph {
    struct AudioContext {
        float sample_freq;
        double clock; // Current elapsed time in seconds
    };

    template<size_t IN, size_t OUT>
    struct InputOutput {
        float inputs[IN];
        float outputs[OUT];
    };

    class Block {
    public:
        double last_processed_time;

        virtual size_t input_size() = 0;

        virtual size_t output_size() = 0;

        virtual float *inputs() = 0;

        virtual float *outputs() = 0;

        virtual void process(AudioContext) = 0;

        virtual std::string name() = 0;

        size_t id() const { return _id; }

    protected:
        size_t _id;

        Block() : last_processed_time(0.0) { _id = ++_latest_id; }

    private:
        static size_t _latest_id;
    };

    struct Wire {
        Block *_in;
        Block *_out;
        size_t in_index;
        size_t in_size;
        size_t out_index;
        size_t out_size;
        size_t _id;

        Wire(Block *in, Block *out,
             size_t in_index, size_t in_size,
             size_t out_index, size_t out_size) : _in(in),
                                                  _out(out),
                                                  in_index(in_index),
                                                  in_size(in_size),
                                                  out_index(out_index),
                                                  out_size(out_size) {
            _id = ++_latest_id;
        }

        size_t id() const { return _id; }

        Wire(const Wire &) = default;               // Copy constructor
        Wire &operator=(const Wire &) = default;

    private:
        static size_t _latest_id;
    };

    class AudioGraph {
    public:
        AudioGraph() {
            _visited.reserve(1000); // Arbitrary number, you can adjust based on your expectations.
            _stack.reserve(1000); // Similarly, an arbitrary number.
        }

        void add_block(std::unique_ptr<Block> block) {
            _blocks[block->id()] = std::move(block);
        }

        void add_wire(const Wire &wire) {
            _wires.insert_or_assign(wire.id(), wire);
        }

        void process(AudioContext ctx);

    private:
        std::unordered_map<size_t, std::unique_ptr<Block> > _blocks;
        std::unordered_map<size_t, Wire> _wires;

        std::unordered_map<Block *, bool> _visited;
        std::vector<Block *> _stack;
        std::vector<Block *> _tempStack;

        void dfs(Block *vertex);
    };

    void AudioGraph::dfs(Block *vertex) {
        _visited[vertex] = true;

        // Visit all the blocks connected via outgoing wires from this block
        for (const auto &wirePair: _wires) {
            const Wire &wire = wirePair.second;
            if (wire._in == vertex && !_visited[wire._out]) {
                dfs(wire._out);
            }
        }

        _stack.push_back(vertex);
    }

    void AudioGraph::process(AudioContext ctx) {
        for (auto &pair: _visited) {
            pair.second = false;
        }
        _stack.clear();

        for (const auto &blockPair: _blocks) {
            if (!_visited[blockPair.second.get()]) {
                dfs(blockPair.second.get());
            }
        }

        // Process the blocks in topological order
        for (Block *block: _stack) {
            if (block->last_processed_time == ctx.clock) {
                continue; // This block has already been processed
            }

            // Copy output values to input values of dependent blocks
            for (const auto &wirePair: _wires) {
                const Wire &wire = wirePair.second;
                if (wire._in == block) {
                    std::copy(block->outputs() + wire.in_index, block->outputs() + wire.in_index + wire.in_size,
                              wire._out->inputs() + wire.out_index);
                }
            }

            // Process the block
            block->process(ctx);
            block->last_processed_time = ctx.clock;
        }
    }
}

#endif //GRAPH_H
