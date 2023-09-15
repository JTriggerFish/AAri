#include "graph.h"
#include "graph_io.h"
#include "audio_engine.h"

namespace Graph {


    void AudioGraph::dfs(Block *startVertex) {

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
        _wires_by_id.clear();

        for (auto &pair: _blocks) {
            _nodeState[pair.second.get()] = UNVISITED;
        }

        // Collect all outgoing wires for each block
        for (const auto &blockPair: _blocks) {
            Block *potentialDependentBlock = blockPair.second.get();
            size_t n;
            const Wire *inputs = potentialDependentBlock->get_input_wires(n);
            for (size_t i = 0; i < n; ++i) {
                const Wire &wire = inputs[i];
                if (wire.in != nullptr) {
                    _outgoingWires.insert(std::make_pair(wire.in, wire));
                    _wires_by_id[wire.id] = const_cast<Wire *>(&wire);
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
            const Wire *in_conections = block->get_input_wires(n);
            // Copy inputs
            for (size_t i = 0; i < n; ++i) {
                const Wire &wire = in_conections[i];
                if (wire.in == nullptr) continue;
                //ASSERT(wire.out_index + wire.width <= block->input_size());
                wire.transmit(wire.in->outputs(), block->inputs());
            }

            // Process the block
            block->process(ctx);
            block->last_processed_time = ctx.clock;
        }
    }

    AudioGraph::AudioGraph(AudioEngine *audioEngine) : _audioEngine(audioEngine) {
        _visited.reserve(256); // Arbitrary number, you can adjust based on your expectations.
        _nodeState.reserve(256); // Arbitrary number, you can adjust based on your expectations.
        _outgoingWires.reserve(256);
        _wires_by_id.reserve(512);
    }


    void AudioGraph::add_block(const std::shared_ptr<Block> &block) {
        auto id = block->id();
        _blocks[id] = block;
        // Note even though the blocks are not connected, blocks on the graph are expected
        // to be processed so we need to update the ordering
        auto guard = _audioEngine->lock_till_function_returns();
        update_ordering();
    }

    void AudioGraph::remove_block(const size_t block_id) {
        auto it = _blocks.find(block_id);
        if (it == _blocks.end())
            throw std::runtime_error("Block not found");
        auto chopping_block = it->second.get();
        auto guard = _audioEngine->lock_till_function_returns();
        chopping_block->clear_wires();
        // The block owns its wires to other blocks so these are going automatically, but
        // we also need to
        // remove wires connecting to this block
        _blocks.erase(it);
        for (auto &block: _blocks) {
            size_t n;
            const Wire *wires = block.second->get_input_wires(n);
            for (size_t i = 0; i < n; ++i) {
                if (wires[i].in == chopping_block) {
                    block.second->disconnect_wire(wires[i].id, true);
                }
            }
        }
        update_ordering();
    }

    size_t AudioGraph::connect_wire(size_t in_block_id, size_t out_block_id,
                                    size_t in_index,
                                    size_t width,
                                    size_t out_index,
                                    float gain,
                                    float offset,
                                    WireTransform transform,
                                    float wire_transform_param) {
        auto out = _blocks[out_block_id].get();
        auto in = _blocks[in_block_id].get();
        auto guard = _audioEngine->lock_till_function_returns();
        auto id = out->connect_wire(in, in_index, width, out_index, gain, offset, transform, wire_transform_param);

        update_ordering();
        return id;
    }

    void AudioGraph::disconnect_wire(size_t wire_id, std::optional<size_t> out_block_id) {
        // If the out block is not specified, find it first:
        auto wires_owner = out_block_id.has_value() ? _blocks[out_block_id.value()].get() : find_wire_owner(wire_id);
        if (wires_owner == nullptr)
            throw std::runtime_error("Wire is not connected to any block");
        auto guard = _audioEngine->lock_till_function_returns();
        wires_owner->disconnect_wire(wire_id, true);
        update_ordering();
    }

    Block *AudioGraph::find_wire_owner(size_t wire_id) {
        if (wire_id == 0) return nullptr; // Invalid wire id
        Block *wires_owner = nullptr;
        for (auto &block: _blocks) {
            size_t n;
            const Wire *wires = block.second->get_input_wires(n);
            for (size_t i = 0; i < n; ++i) {
                if (wires[i].id == wire_id) {
                    wires_owner = block.second.get();
                    break;
                }
            }
        }
        return wires_owner;
    }

    Wire *AudioGraph::find_wire(size_t wire_id) {
        auto it = _wires_by_id.find(wire_id);
        if (it == _wires_by_id.end())
            return nullptr;
        return it->second;
    }

    pybind11::array_t<float> AudioGraph::py_get_block_inputs(size_t block_id, size_t input_index, size_t width) {
        Block *block;
        if (get_block(block_id, &block)) {
            size_t n = block->input_size();
            if (input_index + width > n)
                throw std::runtime_error("Invalid input index or width");
            auto guard = _audioEngine->lock_till_function_returns();
            std::vector<float> ret = {block->inputs() + input_index, block->inputs() + input_index + width};
            return pybind11::array_t<float>(ret.size(), ret.data());
        }
        throw std::runtime_error("Block not found " + std::to_string(block_id));
    }

    void AudioGraph::py_set_block_inputs(size_t block_id, size_t input_index, pybind11::array_t<float> input) {
        Block *block;
        if (!get_block(block_id, &block))
            throw std::runtime_error("Block not found " + std::to_string(block_id));
        size_t n = block->input_size();
        if (input_index + input.size() > n)
            throw std::runtime_error("Invalid input index or width");
        auto guard = _audioEngine->lock_till_function_returns();
        std::memcpy(block->inputs() + input_index, input.data(), input.size() * sizeof(float));
    }

    pybind11::array_t<float> AudioGraph::py_get_block_outputs(size_t block_id, size_t output_index, size_t width) {
        Block *block;
        if (get_block(block_id, &block)) {
            size_t n = block->output_size();
            if (output_index + width > n)
                throw std::runtime_error("Invalid output index or width");
            auto guard = _audioEngine->lock_till_function_returns();
            std::vector<float> ret = {block->outputs() + output_index, block->outputs() + output_index + width};
            return pybind11::array_t<float>(ret.size(), ret.data());
        }
        throw std::runtime_error("Block not found " + std::to_string(block_id));
    }

    void AudioGraph::tweak_wire_gain(size_t wire_id, float gain) {
        auto wire = find_wire(wire_id);
        if (wire == nullptr)
            throw std::runtime_error("Wire not found");
        auto guard = _audioEngine->lock_till_function_returns();
        wire->gain = gain;
    }

    void AudioGraph::tweak_wire_offset(size_t wire_id, float offset) {
        auto wire = find_wire(wire_id);
        if (wire == nullptr)
            throw std::runtime_error("Wire not found");
        auto guard = _audioEngine->lock_till_function_returns();
        wire->offset = offset;

    }

    void AudioGraph::tweak_wire_param(size_t wire_id, float param) {
        auto wire = find_wire(wire_id);
        if (wire == nullptr)
            throw std::runtime_error("Wire not found");
        auto guard = _audioEngine->lock_till_function_returns();
        wire->wire_transform_param = param;
    }

    AudioGraph::~AudioGraph() {
    }


}