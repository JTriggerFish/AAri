//
//

#ifndef AARI_BLOCKS_H
#define AARI_BLOCKS_H

namespace Graph {
    enum class BlockType {
        NONE,
        SineOsc,
        TriangOsc,
        SawOsc,
        SquareOsc,
        Product,
        Sum,
        Constant,
        Mixer,
    };
    struct Block {
        std::vector<entt::entity> inputIds = {
                entt::null, entt::null, entt::null, entt::null,
                entt::null, entt::null, entt::null, entt::null};

        std::vector<entt::entity> outputIds = {entt::null, entt::null, entt::null, entt::null};
        BlockType type = BlockType::NONE;
        uint32_t topo_sort_index = 0;
    };
}

#endif //AARI_BLOCKS_H
