//
//

#ifndef AARI_GRAPH_H
#define AARI_GRAPH_H

#include <entt/entt.hpp>
#include <vector>
#include <string>
#include <stack>
#include "parameters.h"
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
            auto block_view = registry.view<Block>();
            for (auto entity: block_view) {
                //Find all inbound wires to this block
                //and transmit the data from upstream blocks
                auto wire_view = registry.view<Wire>();
                auto &wires_to_block = registry.get<WiresToBlock>(entity);
                for (auto wire_id: wires_to_block.input_wire_ids) {
                    if (wire_id == entt::null)
                        break;
                    auto &wire = wire_view.get<Wire>(wire_id);
                    wire.transmitFunc(registry, wire);
                }

                // Now the block's inputs are up-to-date and we can
                // process it
                auto &block = block_view.get<Block>(entity);
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
