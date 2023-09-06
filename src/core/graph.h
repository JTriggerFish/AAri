#ifndef GRAPH_H
#define GRAPH_H

#include <SDL2/SDL.h>
#include <list>
#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>
#include <stdexcept>

#include <optional>
#include "wire.h"
#include "block.h"
#include <pybind11/numpy.h>


#define ASSERT(condition) \
    if (!(condition)) { \
        throw std::runtime_error("Assertion failed: " #condition); \
    }

namespace Graph {
    class AudioGraph {

    public:
        AudioGraph(int audioDevice);

        ~AudioGraph() {
            unlock();
        }

        // Topology modifying functions:
        void add_block(const std::shared_ptr<Block> &block);

        void remove_block(size_t block_id);

        size_t connect_wire(size_t in_block_id, size_t out_block_id,
                            size_t in_index,
                            size_t width,
                            size_t out_index,
                            float gain = 1.0f,
                            float offset = 0.0f,
                            WireTransform transform = WireTransform::NONE,
                            float wire_transform_param = 0.0f);

        void tweak_wire_gain(size_t wire_id, float gain);

        void tweak_wire_offset(size_t wire_id, float offset);

        void tweak_wire_param(size_t wire_id, float param);

        void disconnect_wire(size_t wire_id, std::optional<size_t> out_block_id = std::nullopt);

        // Processing functions:
        void process(AudioContext ctx);

        bool has_block(size_t block_id) {
            return _blocks.find(block_id) != _blocks.end();
        }

        bool get_block(size_t block_id, Block **block) {
            auto it = _blocks.find(block_id);
            if (it != _blocks.end()) {
                *block = it->second.get();
                return true;
            }
            return false;
        }

        std::unordered_map<size_t, std::shared_ptr<Block> > py_get_all_blocks() {
            return _blocks;
        }

        std::vector<Block *> py_get_topological_order() {
            return {_topologicalOrder.begin(), _topologicalOrder.end()};
        }

        pybind11::array_t<float> py_get_block_inputs(size_t block_id, size_t input_index, size_t width);

        pybind11::array_t<float> py_get_block_outputs(size_t block_id, size_t output_index, size_t width);


        enum NodeState {
            UNVISITED,
            VISITING,
            VISITED
        };

    private:
        std::unordered_map<size_t, std::shared_ptr<Block> > _blocks;
        std::list<Block *> _topologicalOrder;
        int _audioDevice;
        bool _locked;


        void dfs(Block *vertex);

        void update_ordering();

        Block *find_wire_owner(size_t wire_id);

        Wire *find_wire(size_t wire_id);

        void lock() {
            if (_audioDevice >= 0) {
                SDL_LockAudioDevice(_audioDevice);
                _locked = true;
            }
        }

        void unlock() const {
            if (_locked)
                SDL_UnlockAudioDevice(_audioDevice);
        }

    private:
        std::unordered_map<size_t, Wire *> _wires_by_id;
        // Temp memory
        std::unordered_map<Block *, bool> _visited;
        std::unordered_multimap<Block *, Wire> _outgoingWires;
        std::unordered_map<Block *, NodeState> _nodeState;

    };


}

#endif //GRAPH_H
