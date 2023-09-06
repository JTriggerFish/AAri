//

#ifndef RELEASE_WIRE_H
#define RELEASE_WIRE_H

#include <cstddef>

namespace Graph {
    class Block;

    struct Wire {
        Graph::Block *in;
        Graph::Block *out;
        size_t in_index;
        size_t width;
        size_t out_index;
        size_t id;

        Wire(Graph::Block *in, Graph::Block *out,
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

}
#endif //RELEASE_WIRE_H
