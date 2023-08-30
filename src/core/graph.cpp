#include "graph.h"
#include "graph_io.h"

namespace Graph {
    size_t Block::_latest_id = 0;
    size_t Wire::_latest_id = 0;

    
void AudioGraph::dfs(Block *startVertex) {
    std::lock_guard<std::mutex> lock(graphMutex);

        std::stack<Block *> dfsStack;
        dfsStack.push(startVertex);

        while (!dfsStack.empty()) {
            Block *current = dfsStack.top();
            dfsStack.pop();

            if (_nodeState[current] == VISITING) {
                // We've finished processing this node and its descendants
                _nodeState[current] = VISITED;
                _topologicalOrder.push_front(current);
            } else if (_nodeState[current] == UNVISITED) {
                _nodeState[current] = VISITING;
                dfsStack.push(current);  // Push it back to process it again after its descendants

                auto wireRange = _outgoingWires.equal_range(current);
                for (auto wireIt = wireRange.first; wireIt != wireRange.second; ++wireIt) {
                    const Wire &wire = wireIt->second;
                    if (_nodeState[wire.out] == UNVISITED) {
                        dfsStack.push(wire.out);
                    } else if (_nodeState[wire.out] == VISITING) {
                        // Cycle detected
                        throw std::runtime_error("Cycle detected in the audio graph!");
                    }
                }
            }
        }
    }

    void AudioGraph::update_ordering() {
        _topologicalOrder.clear();
        _outgoingWires.clear();
        _nodeState.clear();

        for (auto &pair: _blocks) {
            _nodeState[pair.second.get()] = UNVISITED;
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
            if (_nodeState[blockPair.second.get()] == UNVISITED) {
                dfs(blockPair.second.get());
            }
        }
    }

    void AudioGraph::process(AudioContext ctx) {

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
                if (wire.in == nullptr) continue;
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

    AudioGraph::AudioGraph() {
        _visited.reserve(256); // Arbitrary number, you can adjust based on your expectations.
        _nodeState.reserve(256); // Arbitrary number, you can adjust based on your expectations.
        _outgoingWires.reserve(256);
    }

    void AudioGraph::add_block(const std::shared_ptr<Block> &block) {
        _blocks[block->id()] = block;
        update_ordering();
    }

    void AudioGraph::remove_block(const size_t block_id) {
        auto it = _blocks.find(block_id);
        if (it != _blocks.end()) {
            auto chopping_block = it->second.get();
            // 1. Remove wires connecting to this block
            for (auto &block: _blocks) {
                size_t n;
                Wire *wires = block.second->get_input_wires(n);
                for (size_t i = 0; i < n; ++i) {
                    if (wires[i].in == chopping_block) {
                        block.second->disconnect_wire(wires[i].id, true);
                    }
                }
            }
            _blocks.erase(it);
        }

        update_ordering();
    }

    void
    AudioGraph::connect_wire(size_t in_block_id, size_t out_block_id, size_t in_index, size_t width, size_t out_index) {
        auto out = _blocks[out_block_id].get();
        auto in = _blocks[in_block_id].get();
        out->connect_wire(in, in_index, width, out_index);

        update_ordering();
    }

    void AudioGraph::disconnect_wire(size_t wire_id, std::optional<size_t> out_block_id) {
        // If the out block is not specified, find it first:
        auto wires_owner = out_block_id.has_value() ? _blocks[out_block_id.value()].get() : find_wire_owner(wire_id);
        if (wires_owner == nullptr)
            throw std::runtime_error("Wire is not connected to any block");
        wires_owner->disconnect_wire(wire_id, true);

        update_ordering();
    }

    Block *AudioGraph::find_wire_owner(size_t wire_id) {
        if (wire_id == 0) return nullptr; // Invalid wire id
        Block *wires_owner = nullptr;
        for (auto &block: _blocks) {
            size_t n;
            Wire *wires = block.second->get_input_wires(n);
            for (size_t i = 0; i < n; ++i) {
                if (wires[i].id == wire_id) {
                    wires_owner = block.second.get();
                    break;
                }
            }
        }
        return wires_owner;
    }


}