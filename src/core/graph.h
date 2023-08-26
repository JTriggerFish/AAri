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
        AudioGraph() : _ordered(false) {
            _visited.reserve(256); // Arbitrary number, you can adjust based on your expectations.
            _tempStack.reserve(256); // Similarly, an arbitrary number.
            _topologicalOrder.reserve(256)
        }

        void add_block(std::unique_ptr<Block> block) {
            _blocks[block->id()] = std::move(block);
            _ordered = false;
        }

        void add_wire(const Wire &wire) {
            _wires.insert_or_assign(wire.id(), wire);
            _ordered = false;
        }

        void process(AudioContext ctx);

    private:
        std::unordered_map<size_t, std::unique_ptr<Block> > _blocks;
        std::unordered_map<size_t, Wire> _wires;

        std::vector<Block *> _topologicalOrder;
        std::unordered_map<Block *, bool> _visited;

        std::vector<Block *> _tempStack;

        bool _ordered = false;

        void dfs(Block *vertex);

        void update_ordering();
    };

    void AudioGraph::dfs(Block *startVertex) {
        _tempStack.clear();
        _tempStack.push_back(startVertex);

        while (!_tempStack.empty()) {
            Block *current = _tempStack.back();
            _tempStack.pop_back();

            if (!_visited[current]) {
                _visited[current] = true;

                // Add all unvisited neighbors to the temporary stack
                for (const auto &wirePair: _wires) {
                    const Wire &wire = wirePair.second;
                    if (wire._in == current && !_visited[wire._out]) {
                        _tempStack.push_back(wire._out);
                    }
                }
                // After visiting all neighbors, add current node to topological order
                _topologicalOrder.push_back(current);
            }
        }
    }

    void AudioGraph::process(AudioContext ctx) {
        if (!_ordered) {
            update_ordering();
            _ordered = true;
        }

        // Process the blocks in topological order
        for (Block *block: _topologicalOrder) {
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

    void AudioGraph::update_ordering() {
        _topologicalOrder.clear();

        for (auto &pair: _visited) {
            pair.second = false;
        }

        for (const auto &blockPair: _blocks) {
            if (!_visited[blockPair.second.get()]) {
                dfs(blockPair.second.get());
            }
        }

    }
}

#endif //GRAPH_H
