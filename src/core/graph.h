#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <memory>
#include <unordered_map>

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
        virtual size_t input_size() = 0;

        virtual size_t output_size() = 0;

        virtual float *inputs() = 0;

        virtual const float *outputs() = 0;

        virtual void process(AudioContext) = 0;

        virtual std::string name() = 0;

        double last_processed_time() const { return _last_processed_time; }

    protected:
        static size_t _latest_id;
        double _last_processed_time;

        Block() : _last_processed_time(0.0) { ++_latest_id; }
    };

    struct Wire {
        const Block *_in;
        Block *_out;
        size_t in_index;
        size_t in_size;
        size_t out_index;
        size_t out_size;

        Wire(const Block *in, Block *out,
             size_t in_index, size_t in_size,
             size_t out_index, size_t out_size) : _in(in),
                                                  _out(out),
                                                  in_index(in_index),
                                                  in_size(in_size),
                                                  out_index(out_index),
                                                  out_size(out_size) {
            ++_latest_id;
        }

    private:
        static size_t _latest_id;
    };

    class AudioGraph {
    private:
        std::unordered_map<size_t, std::unique_ptr<Block> > _blocks;
        std::unordered_map<size_t, Wire> _wires;
    };
}
#endif //GRAPH_H