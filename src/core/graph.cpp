#include "graph.h"

namespace Graph {
    size_t Block::_latest_id = 1;
    size_t Wire::_latest_id = 1;

    void AudioGraph::dfs(Block *startVertex) {
        _tempStack.clear();
        _tempStack.push_back(startVertex);

        while (!_tempStack.empty()) {
            Block *current = _tempStack.back();
            _tempStack.pop_back();

            if (!_visited[current]) {
                _visited[current] = true;

                // Use our prebuilt multimap to fetch all output wires for the current block efficiently
                auto wireRange = _outgoingWires.equal_range(current);
                for (auto wireIt = wireRange.first; wireIt != wireRange.second; ++wireIt) {
                    const Wire &wire = wireIt->second;
                    if (!_visited[wire.out]) {
                        _tempStack.push_back(wire.out);
                    }
                }

                // After visiting all neighbors, add current node to topological order
                _topologicalOrder.push_back(current);
            }
        }
    }

    void AudioGraph::update_ordering() {
        _topologicalOrder.clear();
        _outgoingWires.clear();
        _visited.clear();

        for (auto &pair: _blocks) {
            _visited[pair.second.get()] = false;
        }

        // Collect all outgoing wires for each block
        for (const auto &blockPair: _blocks) {
            Block *potentialDependentBlock = blockPair.second.get();
            size_t n;
            Wire *inputs = potentialDependentBlock->get_input_wires(n);
            for (size_t i = 0; i < n; ++i) {
                const Wire &wire = inputs[i];
                if (wire.in != nullptr) {
                    _outgoingWires.insert(std::make_pair(wire.in, wire));
                }
            }
        }

        for (const auto &blockPair: _blocks) {
            if (!_visited[blockPair.second.get()]) {
                dfs(blockPair.second.get());
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

            size_t n;
            Wire *inputs = block->get_input_wires(n);
            // Copy inputs
            for (size_t i = 0; i < n; ++i) {
                const Wire &wire = inputs[i];
                ASSERT(wire.in != nullptr);
                ASSERT(wire.out_index + wire.width <= block->input_size());
                std::copy(wire.in->outputs() + wire.in_index,
                          wire.in->outputs() + wire.in_index + wire.width,
                          block->inputs() + wire.out_index);
            }

            // Process the block
            block->process(ctx);
            block->last_processed_time = ctx.clock;
        }
    }

    AudioGraph::AudioGraph() : _ordered(false) {
        _visited.reserve(256); // Arbitrary number, you can adjust based on your expectations.
        _tempStack.reserve(256); // Similarly, an arbitrary number.
        _topologicalOrder.reserve(256);
        _outgoingWires.reserve(256);

        //Disable denormals for performance
        _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
        _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    }

    void AudioGraph::add_block(std::unique_ptr<Block> block) {
        _blocks[block->id()] = std::move(block);
        _ordered = false;
    }

    void AudioGraph::connect(Block *in, Block *out, size_t in_index, size_t width, size_t out_index) {
        out->connect(in, in_index, width, out_index);
        _ordered = false;
    }

    void AudioGraph::disconnect(Block *out, size_t out_index_or_id, bool is_id) {
        out->disconnect(out_index_or_id, is_id);
        _ordered = false;
    }

}