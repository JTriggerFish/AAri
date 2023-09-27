//
//

#ifndef AARI_BLOCKS_H
#define AARI_BLOCKS_H

#include <array>
#include "audio_context.h"


namespace AAri {

    struct Block;

    //typedef process func pointer
    typedef void (*ProcessFunc)(entt::registry &registry, const Block &block, AudioContext ctx);
    //Alternative modern c++ style function pointer:

    struct Visited {
        enum State {
            UNVISITED,
            VISITING,
            VISITED
        };
        State state = UNVISITED;
    };

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
        // Members ----------------------------------------------------------------------
        std::array<entt::entity, 8> inputIds = {
                entt::null, entt::null, entt::null, entt::null,
                entt::null, entt::null, entt::null, entt::null};
        std::array<entt::entity, 4> outputIds = {entt::null, entt::null, entt::null, entt::null};

        BlockType type = BlockType::NONE;
        uint32_t topo_sort_index = 0;

        ProcessFunc processFunc = nullptr;
        // ------------------------------------------------------------------------------

        //Static functions to create and destroy blocks
        static entt::entity
        create(entt::registry &registry, BlockType type, const std::array<entt::entity, 8> &inputIds,
               const std::array<entt::entity, 4> &outputIds, ProcessFunc processFunc) {
            auto entity = registry.create();
            registry.emplace<Block>(entity, inputIds, outputIds, type, 0u, processFunc);
            registry.emplace<Visited>(entity, Visited::UNVISITED);
            return entity;
        }

        static void destroy(entt::registry &registry, entt::entity entity) {
            registry.destroy(entity);
        }
    };

    struct InputExpansion {
        std::array<entt::entity, 8> inputIds = {entt::null, entt::null, entt::null, entt::null,
                                                entt::null, entt::null, entt::null, entt::null};
    };
    struct OutputExpansion {
        std::array<entt::entity, 4> outputIds = {entt::null, entt::null, entt::null, entt::null};
    };
}

#endif //AARI_BLOCKS_H
