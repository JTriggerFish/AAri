//
//

#ifndef AARI_GRAPH_H
#define AARI_GRAPH_H

#include <entt/entt.hpp>
#include <vector>
#include <string>
#include <stack>
#include "inputs_outputs.h"
#include "blocks.h"
#include "wires.h"
#include "utils/data_structures.h"
#include "audio_context.h"

namespace AAri {
    struct ClockLink {
        float speed_multiplier = 1.0;
    };

    class Graph {
    public:
        /**
         * Note : adding wires and blocks is all done by creating entities in this registry.
         */
        entt::registry registry;

        /**
         * Process the graph for one sample
         * @param ctx
         */
        void process(AudioContext ctx) {
            //The blocks are already sorted at this point, so
            //we just need to use the entt functions to iterate through them
            auto block_view = registry.view<Block, WiresToBlock>();
            auto wire_view = registry.view<Wire>();
            for (auto entity: block_view) {
                //Find all inbound wires to this block
                //and transmit the data from upstream blocks
                auto &wires_to_block = block_view.get<WiresToBlock>(entity);
                auto &block = block_view.get<Block>(entity);
                for (auto wire_id: wires_to_block.input_wire_ids) {
                    if (wire_id == entt::null)
                        continue;
                    auto &wire = wire_view.get<Wire>(wire_id);
                    wire.transmitFunc(registry, wire);
                }

                // Now the block's inputs are up-to-date and we can
                // process it
                block.processFunc(registry, block, ctx);
            }
        }

        /**
         * Topological sort of the blocs in the graph
         */
        void toposort_blocks();


    private:

        void dfs(entt::entity block);

        std::vector<entt::entity> _sorted_blocks;
        Stack<entt::entity> _dfs_stack;
    };
}


#endif //AARI_GRAPH_H
