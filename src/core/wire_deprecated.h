//

#ifndef RELEASE_WIRE_H
#define RELEASE_WIRE_H

#include <cstddef>
#include <stdexcept>
#include <cmath>


namespace deprecated_Graph {
    class Block;

    enum class WireTransform {
        NONE,
        EXPAND,
        STEREO_PAN,
    };

    struct Wire {
        size_t id = 0;

        deprecated_Graph::Block *in = nullptr;
        deprecated_Graph::Block *out = nullptr;
        size_t in_index = 0;
        size_t width = 0;
        size_t out_index = 0;

        float gain = 1.0f;
        float offset = 0.0f;
        WireTransform transform = WireTransform::NONE;
        float wire_transform_param = 0.0f;

        Wire(deprecated_Graph::Block *in,
             deprecated_Graph::Block *out,
             size_t in_index,
             size_t width,
             size_t out_index,
             float gain = 1.0f,
             float offset = 0.0f,
             WireTransform transform = WireTransform::NONE,
             float wire_transform_param = 0.0f) :
                id(++_latest_id),
                in(in), out(out),
                in_index(in_index), width(width), out_index(out_index),
                gain(gain), offset(offset), transform(transform),
                wire_transform_param(wire_transform_param) {
            if (transform == WireTransform::STEREO_PAN && width != 2)
                throw std::runtime_error("Stereo pan transform can only be applied to width=2");
        }

        void transmit(const float *input_buffer, float *output_buffer) const {

            if (transform == WireTransform::NONE) {
                transmit_standard(input_buffer, output_buffer);
            } else if (transform == WireTransform::EXPAND) {
                transmit_expand(input_buffer, output_buffer);
            } else if (transform == WireTransform::STEREO_PAN) {
                transmit_stereo_pan(input_buffer, output_buffer);
            }
        }

        void transmit_standard(const float *input_buffer, float *output_buffer) const {
            for (size_t i = 0; i < width; ++i) {
                output_buffer[out_index + i] = input_buffer[in_index + i] * gain + offset;
            }
        }

        void transmit_expand(const float *input_buffer, float *output_buffer) const {
            for (size_t i = 0; i < width; ++i) {
                output_buffer[out_index + i] = input_buffer[in_index] * gain + offset;
            }
        }

        void transmit_stereo_pan(const float *input_buffer, float *output_buffer) const {
            float pan = wire_transform_param;
            float left_gain = gain * sqrtf((1.0f - pan) / 2.0f);
            float right_gain = gain * sqrtf((1.0f + pan) / 2.0f);
            output_buffer[out_index] = input_buffer[in_index] * left_gain + offset;
            output_buffer[out_index + 1] = input_buffer[in_index] * right_gain + offset;
        }

        Wire() = default;


        Wire(const Wire &) = default;               // Copy constructor
        Wire &operator=(const Wire &) = default;

    private:
        static size_t _latest_id;
    };

}
#endif //RELEASE_WIRE_H
