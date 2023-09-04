//
//

#include "block.h"
#include "graph_io.h"

size_t Graph::Block::_latest_id = 0;

std::vector<Wire> Graph::Block::py_get_input_wires() const {
    size_t n;
    auto wires = get_input_wires(n);
    std::vector<Wire> result = {wires, wires + n};
    result.erase(std::remove_if(result.begin(), result.end(), [](const Wire &w) {
        return w.in == nullptr;
    }), result.end());
    return result;
}
