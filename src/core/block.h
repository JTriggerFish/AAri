//
//

#ifndef RELEASE_BLOCK_H
#define RELEASE_BLOCK_H

#include <mutex>
#include <optional>
#include <stdexcept>
#include <algorithm>
#include <stack>
#include <unordered_map>
#include <memory>
#include <vector>
#include <list>
#include <SDL2/SDL.h>

namespace Graph {
    struct AudioContext {
        float sample_freq;
        double clock; // Current elapsed time in seconds
    };

    class Block {

    public:
        friend class AudioGraph;
        virtual size_t input_size() const = 0;

        virtual size_t output_size() const = 0;

        virtual float *inputs() = 0;

        virtual const float *outputs() const = 0;

        virtual std::string name() const = 0;

        virtual void process(AudioContext) = 0;

        virtual ~Block() = default;

        size_t id() const {
            return _id;
        }

        double last_processed_time;
    protected:
        size_t _id;

        Block() : last_processed_time(-1.0) { _id = ++_latest_id; }
    private:
        static size_t _latest_id;

    };


    };

} // Graph

#endif //RELEASE_BLOCK_H
