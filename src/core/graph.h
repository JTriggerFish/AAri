#ifndef GRAPH_H
#define GRAPH_H

#include <list>
#include <vector>
#include <memory>
#include <unordered_map>
#include <stack>
#include <algorithm>
#include <stdexcept>

#include <optional>
#include <mutex>


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

        virtual float *inputs() = 0;

        virtual const float *outputs() const = 0;

        virtual std::string name() const = 0;

        size_t id() const { return _id; }

        virtual ~Block() = default;

    protected:
        size_t _id;

        Block() : last_processed_time(-1.0) { _id = ++_latest_id; }

        virtual void process(AudioContext) = 0;

        virtual Wire *get_input_wires(size_t &size) = 0;

        virtual void connect_wire(Block *in, size_t in_index, size_t width, size_t out_index) = 0;

        virtual void disconnect_wire(size_t out_index_or_id, bool is_id) = 0;

    private:
        static size_t _latest_id;
    };


    class AudioGraph {
    public:
        std::mutex graphMutex;

    public:
        AudioGraph();

        // Topology modifying functions:
        void add_block(const std::shared_ptr<Block> &block);

        void remove_block(size_t block_id);

        void connect_wire(size_t in_block_id, size_t out_block_id, size_t in_index, size_t width, size_t out_index);

        void disconnect_wire(size_t wire_id, std::optional<size_t> out_block_id = std::nullopt);

        // Processing functions:
        void process(AudioContext ctx);

        bool get_block(size_t block_id, Block **block) {
            auto it = _blocks.find(block_id);
            if (it != _blocks.end()) {
                *block = it->second.get();
                return true;
            }
            return false;
        }


        enum NodeState {
            UNVISITED,
            VISITING,
            VISITED
        };

    private:
        std::unordered_map<size_t, std::shared_ptr<Block> > _blocks;
        std::list<Block *> _topologicalOrder;

        void dfs(Block *vertex);

        void update_ordering();

        Block *find_wire_owner(size_t wire_id);

    private:
        // Temp memory
        std::unordered_map<Block *, bool> _visited;
        std::unordered_multimap<Block *, Wire> _outgoingWires;
        std::unordered_map<Block *, NodeState> _nodeState;

    };


}

#endif //GRAPH_H
