//
//

#include "graph.h"

void AAri::Graph::toposort_blocks() {
    //First get all the blocks and all the wires from the registry:
    auto blocks = registry.view<Block>();
    auto wires = registry.view<Wire>();

    //Now do a classic topological sort, keeping the block ids in a vector:
    _sorted_blocks.reserve(blocks.size());
    _sorted_blocks.clear();
    _dfs_stack.reserve(blocks.size());
    _dfs_stack.clear();

    //First mark all blocks as unvisited:
    registry.view<Visited>().each([](auto &visited) {
        visited.state = Visited::UNVISITED;
    });
    // Do a dfs on the blocks:
    for (auto block: blocks) {
        if (registry.get<Visited>(block).state == Visited::UNVISITED) {
            dfs(block);
        }
    }
    //Note that _sorted_blocks is in reverse order.
    //1) Set the block's topo_sort_index based on the order in _sorted_blocks:
    for (int i = 0; i < _sorted_blocks.size(); ++i) {
        registry.get<Block>(_sorted_blocks[i]).topo_sort_index = i;
    }
    //2) sort the blocks in the registry based on topo_sort_index, but in reverse order:
    registry.sort<Block>([](const Block &lhs, const Block &rhs) {
        return lhs.topo_sort_index > rhs.topo_sort_index;
    });
    //3) Update each block's WiresToBlock struct:
    registry.view<WiresToBlock>().each([&](auto id, auto &wires_to_block) {
        //First clear the array:
        for (auto &input_id: wires_to_block.input_wire_ids) {
            input_id = entt::null;
        }
        //
        //Now fill it with the wires connected to this block:
        wires.each([&](auto wire_id, auto &wire) {
            if (wire.to_block == id) {
                //Find the first available slot in the array and throw an error if there is
                //none:
                bool found = false;
                for (auto &input_id: wires_to_block.input_wire_ids) {
                    if (input_id == entt::null) {
                        input_id = wire_id;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    throw std::runtime_error("Too many wires. we only support 16 wires to each block at present");
                }
            }
        });
    });
    //4) Also sort the wires so that if block i is before block j in the registry,
    //then all the input wires to block i are before all the input wires to block j
    //This should significantly improve cache coherence
    registry.sort<Wire>([this](const Wire &lhs, const Wire &rhs) {
        //Use the blocks topo sort index
        auto lhs_block = lhs.to_block;
        auto rhs_block = rhs.to_block;
        return registry.get<Block>(lhs_block).topo_sort_index >
               registry.get<Block>(rhs_block).topo_sort_index;
    });


}

void AAri::Graph::dfs(entt::entity block) {
    _dfs_stack.clear();
    _dfs_stack.push(block);

    while (!_dfs_stack.empty()) {
        auto current_block = _dfs_stack.pop();
        auto &visit = registry.get<Visited>(current_block);
        if (visit.state == Visited::VISITING) {
            //We have finished visiting this block and all its children
            visit.state = Visited::VISITED;
            _sorted_blocks.push_back(current_block);
        } else if (visit.state == Visited::UNVISITED) {
            //We are visiting this block for the first time
            visit.state = Visited::VISITING;
            _dfs_stack.push(current_block); // Push back to process after all children
            //Now push all the children of this block on the stack
            //Get all the wires that start from this block:
            registry.view<Wire>().each([&](auto id, auto &wire) {
                if (wire.from_block == current_block) {
                    auto target_state = registry.get<Visited>(wire.to_block).state;
                    if (target_state == Visited::UNVISITED)
                        _dfs_stack.push(wire.to_block);
                    else if (target_state == Visited::VISITING)
                        throw std::runtime_error("Cycle detected in the graph!");
                }
            });
        }
    }
}
