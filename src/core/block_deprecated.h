//
//

#ifndef RELEASE_BLOCK_H
#define RELEASE_BLOCK_H

#include <string>
#include "wire_deprecated.h"

namespace deprecated_Graph {

    struct AudioContext {
        float sample_freq;
        double clock; // Current elapsed time in seconds
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

        virtual void clear_wires() = 0;

        size_t id() const { return _id; }

        virtual ~Block() = default;

        std::vector<Wire> py_get_input_wires() const;

    protected:
        size_t _id;

        Block() : last_processed_time(-1.0) { _id = ++_latest_id; }

        virtual void process(AudioContext) = 0;

        virtual const Wire *get_input_wires(size_t &size) const = 0;

        virtual size_t connect_wire(Block *in, size_t in_index, size_t width, size_t out_index,
                                    float gain = 1.0f,
                                    float offset = 0.0f,
                                    WireTransform transform = WireTransform::NONE,
                                    float wire_transform_param = 0.0f) = 0;

        virtual void disconnect_wire(size_t out_index_or_id, bool is_id) = 0;

    private:
        static size_t _latest_id;
    };

}

#endif //RELEASE_BLOCK_H
