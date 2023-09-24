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

namespace AAri {
    struct AudioContext {
        float sample_freq;
        float dt;
        double clock; // Current elapsed time in seconds
    };
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
         * Topological sort of the blocs in the graph
         */
        void toposort_blocks();

        void dfs(entt::entity block);

    private:
        std::vector<entt::entity> _sorted_blocks;
        Stack<entt::entity> _dfs_stack;
    };
}


#endif //AARI_GRAPH_H
