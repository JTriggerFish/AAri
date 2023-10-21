//
//

#ifndef AARI_BLOCKS_H
#define AARI_BLOCKS_H

#include <array>
#include "audio_context.h"
#include "utils/data_structures.h"
#include <entt/entt.hpp>


namespace AAri {
    constexpr int N_INPUTS = 8;
    constexpr int N_OUTPUTS = 4;
    constexpr int N_WIRES = 16;

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
        TriOsc,
        SawOsc,
        SquareOsc,
        Product,
        Sum,
        Constant,
        MonoMixer,
        StereoMixer,
    };
    struct WiresToBlock {
        /** Record wires incoming to block in order to avoid to find all wires
         * connected to a block every time we run the real time loop
         * Max of 16 input wires per block at present.
         * Wires ids have to be sequential: a null means the end of the list
         *
         */
        std::array<entt::entity, N_WIRES> input_wire_ids = fill_with_null<N_WIRES>();
    };

    struct Block {
        friend class AudioEngine;

        // Members ----------------------------------------------------------------------
        std::array<entt::entity, N_INPUTS> inputIds = fill_with_null<N_INPUTS>();
        std::array<entt::entity, N_OUTPUTS> outputIds = fill_with_null<N_OUTPUTS>();

        BlockType type = BlockType::NONE;
        uint32_t topo_sort_index = 0;

        ProcessFunc processFunc = nullptr;
        // ------------------------------------------------------------------------------

        //Static functions to create and destroy blocks
        static entt::entity
        create(entt::registry &registry, BlockType type, const std::array<entt::entity, N_INPUTS> &inputIds,
               const std::array<entt::entity, N_OUTPUTS> &outputIds, ProcessFunc processFunc) {
            auto entity = registry.create();
            registry.emplace<Block>(entity, inputIds, outputIds, type, 0u, processFunc);
            registry.emplace<Visited>(entity, Visited::UNVISITED);
            registry.emplace<WiresToBlock>(entity);
            return entity;
        }

    private:
        // Deletion is private because it requires a new topological sort of the graph
        // therefore should be done through the AudioEngine class
        static void destroy(entt::registry &registry, entt::entity entity) {
            //Delete all the inputs and outputs:
            auto &block = registry.get<Block>(entity);
            for (auto input: block.inputIds) {
                if (input != entt::null) {
                    registry.destroy(input);
                }
            }
            for (auto output: block.outputIds) {
                if (output != entt::null) {
                    registry.destroy(output);
                }
            }
            registry.destroy(entity);
        }
    };

    struct InputExpansion {
        std::array<entt::entity, N_INPUTS> inputIds = fill_with_null<N_INPUTS>();
    };
    struct OutputExpansion {
        std::array<entt::entity, N_OUTPUTS> outputIds = fill_with_null<N_OUTPUTS>();
    };
}

#endif //AARI_BLOCKS_H
