#ifndef GRAPH_H
#define GRAPH_H
#include <vector>
#include <memory>

namespace Graph
{
    struct AudioContext
    {
        float sample_freq;
        double clock; // Current elapsed time in seconds
    };
    template <size_t IN, size_t OUT>
    struct InputOutput
    {
        float[IN] inputs;
        float[OUT] outputs;
    };
    class Block
    {
    public:
        virtual size_t input_size() = 0;
        virtual size_t output_size() = 0;
        virtual float *inputs() = 0;
        virtual const float *outtputs() = 0;

        virtual void process(AudioContext) = 0;
    };
    class AudioGraph
    {
    private:
        std::vector<std::unique_ptr<Block>> blocks;
    };
}
#endif GRAPH_H