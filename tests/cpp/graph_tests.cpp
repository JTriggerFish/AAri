#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include "../../src/core/inputs_outputs.h"
#include "../../src/core/graph.h"
#include "../../src/core/audio_engine.h"
#include "../../src/blocks/mixers.h"
#include "../../src/blocks/oscillators.h"
#include <entt/entt.hpp>
#include <catch2/catch_all.hpp>

#include <thread>

using namespace AAri;

entt::entity getOutputId(entt::registry&registry, entt::entity blockId, int outputIndex) {
    auto&block = registry.get<Block>(blockId);
    return block.outputIds[outputIndex];
}

entt::entity getInputId(entt::registry&registry, entt::entity blockId, int inputIndex) {
    auto&block = registry.get<Block>(blockId);
    return block.inputIds[inputIndex];
}

void times_two(entt::registry&registry, const Block&block, AudioContext ctx) {
    auto&input = registry.get<Input1D>(block.inputIds[0]);
    auto&output = registry.get<Output1D>(block.outputIds[0]);
    output.value = input.value * 2.0f;
}

void plus_three(entt::registry&registry, const Block&block, AudioContext ctx) {
    auto&input = registry.get<Input1D>(block.inputIds[0]);
    auto&output = registry.get<Output1D>(block.outputIds[0]);
    output.value = input.value + 3.0f;
}

void times_2_and_plus_4(entt::registry&registry, const Block&block, AudioContext ctx) {
    auto&input1 = registry.get<Input1D>(block.inputIds[0]);
    auto&input2 = registry.get<Input1D>(block.inputIds[1]);
    auto&output1 = registry.get<Output1D>(block.outputIds[0]);
    auto&output2 = registry.get<Output1D>(block.outputIds[1]);
    output1.value = input1.value * 2.0f;
    output2.value = input2.value + 4.0f;
}

void times_2_and_plus_4_vectorized(entt::registry&registry, const Block&block, AudioContext ctx) {
    auto&input = registry.get<Input2D>(block.inputIds[0]);
    auto&output = registry.get<Output2D>(block.outputIds[0]);
    output.value[0] = input.value[0] * 2.0f;
    output.value[1] = input.value[1] + 4.0f;
}

entt::entity create_times_two(entt::registry&registry) {
    auto input = registry.create();
    registry.emplace<Input1D>(input, 0.0f);
    auto output = registry.create();
    registry.emplace<Output1D>(output, 0.0f);

    return Block::create(registry, BlockType::Product,
                         fill_with_null<N_INPUTS>(input),
                         fill_with_null<N_OUTPUTS>(output),
                         times_two, nullptr);
}

entt::entity create_plus_three(entt::registry&registry) {
    auto input = registry.create();
    registry.emplace<Input1D>(input, 0.0f);
    auto output = registry.create();
    registry.emplace<Output1D>(output, 0.0f);

    return Block::create(registry, BlockType::Sum,
                         fill_with_null<N_INPUTS>(input),
                         fill_with_null<N_OUTPUTS>(output),
                         plus_three, nullptr);
}

entt::entity create_times_2_and_plus_4(entt::registry&registry) {
    auto input1 = registry.create();
    registry.emplace<Input1D>(input1, 0.0f);
    auto input2 = registry.create();
    registry.emplace<Input1D>(input2, 0.0f);
    auto output1 = registry.create();
    registry.emplace<Output1D>(output1, 0.0f);
    auto output2 = registry.create();
    registry.emplace<Output1D>(output2, 0.0f);

    return Block::create(registry, BlockType::Sum,
                         fill_with_null<N_INPUTS>(input1, input2),
                         fill_with_null<N_OUTPUTS>(output1, output2),
                         times_2_and_plus_4, nullptr);
}

entt::entity create_times_2_and_plus_4_vectorized(entt::registry&registry) {
    auto input = registry.create();
    registry.emplace<Input2D>(input, std::array<float, 2>{0.0f, 0.0f});
    auto output = registry.create();
    registry.emplace<Output2D>(output, std::array<float, 2>{0.0f, 0.0f});

    return Block::create(registry, BlockType::Sum,
                         fill_with_null<N_INPUTS>(input),
                         fill_with_null<N_OUTPUTS>(output),
                         times_2_and_plus_4_vectorized, nullptr);
}


TEST_CASE("Testing audio start stop with graph callback", "[AudioEngine]") {
    AudioEngine engine;

    SECTION("Testing start and stop") {
        engine.startAudio();
        auto osc = SineOsc::create(&engine, 440.0f, 1.0f);
        auto osc_block = engine.view_block(osc);
        auto output_mixer = StereoMixer<2>::create(&engine);
        auto mixer_block = engine.view_block(output_mixer);
        auto output_id = mixer_block.outputIds[0];

        engine.set_output_ref(output_id, 2);
        //Wire oscillator to output :
        engine.add_wire_to_mixer(osc, output_mixer, osc_block.outputIds[0], 0,
                                 Wire::transmit_mono_to_stereo_mixer<2>);
        //Start and stop audio:
        //Sleep for 3 seconds:
        std::this_thread::sleep_for(std::chrono::seconds(3));
        auto out1 = engine.view_block_io(osc);
        auto out2 = engine.view_block_io(osc);
        engine.stopAudio();
    }
}

// Unit tests
TEST_CASE("Testing AudioGraph with Dummy Blocks", "[AudioGraph]") {
    AudioEngine engine;
    auto [registry, guard] = engine.get_graph_registry();
    auto block1 = create_plus_three(registry);
    auto block2 = create_times_two(registry);

    AudioContext ctx{44100.0f, 1.0f / 44100.0f, 0.1};
    auto&graph = engine._test_only_get_graph();


    SECTION("Adding and processing blocks") {
        //Set the input of block 2 to 1 :
        registry.get<Input1D>(registry.get<Block>(block2).inputIds[0]).value = 1.0f;

        //Process one sample on the graph
        graph.process(ctx);
        //Get the output of the blocks
        auto&output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto&output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);

        REQUIRE(output1.value == 3.0f);
        REQUIRE(output2.value == 2.0f);
    }

    // Note that each section rebuilds the graph, so the ids will be different
    SECTION("Testing connection") {
        //Connecting the blocks and processing again
        auto&output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto&output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);

        // Set block 2 input to 2
        registry.get<Input1D>(registry.get<Block>(block2).inputIds[0]).value = 2.0f;

        engine.add_wire(block2, block1, getOutputId(registry, block2, 0),
                        getInputId(registry, block1, 0), Wire::transmit_1d_to_1d);

        //Process one sample on the graph
        graph.process(ctx);

        REQUIRE(output2.value == 4.0f);
        REQUIRE(output1.value == 7.0f);

        //Adding the same wire again should throw an exception
        REQUIRE_THROWS(engine.add_wire(block2, block1, getOutputId(registry, block2, 0),
            getInputId(registry, block1, 0), Wire::transmit_1d_to_1d));
    }

    SECTION("Testing disconnection") {
        engine.add_wire(block2, block1, getOutputId(registry, block2, 0),
                        getInputId(registry, block1, 0), Wire::transmit_1d_to_1d);

        auto maybe_wire = engine.get_wire_to_input(registry.get<Block>(block1).inputIds[0]);
        REQUIRE(maybe_wire.has_value());
        REQUIRE(engine.get_wires_to_block(block1).size() == 1);
        REQUIRE(engine.get_wires_to_block(block1)[0] == maybe_wire.value());
        REQUIRE(engine.get_wires_from_block(block2).size() == 1);
        REQUIRE(engine.get_wires_from_block(block2)[0] == maybe_wire.value());

        // Remove
        engine.remove_wire(maybe_wire.value());
        // Wire is now gone
        REQUIRE(!engine.get_wire_to_input(registry.get<Block>(block1).inputIds[0]).has_value());
        //No wires on block 1 or two now:
        REQUIRE(engine.get_wires_from_block(block1).empty());
        REQUIRE(engine.get_wires_to_block(block1).empty());
        REQUIRE(engine.get_wires_to_block(block2).empty());
        REQUIRE(engine.get_wires_from_block(block1).empty());

        //Set the input of block 2 to 1 :
        registry.get<Input1D>(registry.get<Block>(block2).inputIds[0]).value = 1.0f;
        //Process one sample on the graph
        graph.process(ctx);
        //Get the output of the blocks
        auto&output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto&output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);

        // Back to disconnected expected values
        REQUIRE(output1.value == 3.0f);
        REQUIRE(output2.value == 2.0f);
    }
    SECTION("Testing wire gain and offset") {
        auto wire = engine.add_wire(block2, block1, getOutputId(registry, block2, 0),
                                    getInputId(registry, block1, 0), Wire::transmit_1d_to_1d);

        // Check gain and offset are as expected:
        REQUIRE(engine.view_wire(wire).gain == 1.0f);
        REQUIRE(engine.view_wire(wire).offset == 0.0f);

        // Set wire gain to zero:
        engine.tweak_wire_gain(wire, 0.0f);
        REQUIRE(engine.view_wire(wire).gain == 0.0f);
        REQUIRE(engine.view_wire(wire).offset == 0.0f);

        //Check offset tweaking then set it back
        engine.tweak_wire_offset(wire, 1.0f);
        REQUIRE(engine.view_wire(wire).gain == 0.0f);
        REQUIRE(engine.view_wire(wire).offset == 1.0f);
        engine.tweak_wire_offset(wire, 0.0f);

        //Set the input of block 2 to 1 :
        registry.get<Input1D>(registry.get<Block>(block2).inputIds[0]).value = 1.0f;
        //Process one sample on the graph
        graph.process(ctx);
        //Get the output of the blocks
        auto&output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto&output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);

        // Back to disconnected expected values
        REQUIRE(output1.value == 3.0f);
        REQUIRE(output2.value == 2.0f);
    }
}


TEST_CASE("Additional Testing of AudioGraph with Multiple Scenarios", "[AudioGraph]") {
    AudioEngine engine;
    auto [registry, guard] = engine.get_graph_registry();
    auto block1 = create_times_two(registry);
    auto block2 = create_plus_three(registry);
    auto block3 = create_times_2_and_plus_4(registry);

    AudioContext ctx{48000.0f, 1.0f / 48000.0f, 0.5};
    auto&graph = engine._test_only_get_graph();


    SECTION("Testing Multiple layers of dependencies") {
        //Connect block 1's output to block 3's first input
        //and block 3's first output to block 2's input
        engine.add_wire(block1, block3, getOutputId(registry, block1, 0),
                        getInputId(registry, block3, 0), Wire::transmit_1d_to_1d);
        engine.add_wire(block3, block2, getOutputId(registry, block3, 0),
                        getInputId(registry, block2, 0), Wire::transmit_1d_to_1d);

        //Set block 1 input to 2
        registry.get<Input1D>(registry.get<Block>(block1).inputIds[0]).value = 2.0f;
        // Process one sample on the graph
        graph.process(ctx);
        //Get the output of the blocks
        auto&output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto&output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);
        auto&output3_0 = registry.get<Output1D>(registry.get<Block>(block3).outputIds[0]);
        auto&output3_1 = registry.get<Output1D>(registry.get<Block>(block3).outputIds[1]);

        REQUIRE(output1.value == 4.0f);
        REQUIRE(output3_0.value == 8.0f);
        REQUIRE(output3_1.value == 4.0f);

        REQUIRE(output2.value == 11.0f);


        //Now get the order of all the wires and check that they
        //are stored in the same order that the blocks are processed:
        //We do this by putting them in a vector first:
        std::vector<entt::entity> wires;
        for (auto wire: registry.view<Wire>()) {
            wires.push_back(wire);
        }
        //Get block order:
        std::vector<entt::entity> blocks;
        for (auto block: registry.view<Block>()) {
            blocks.push_back(block);
        }
        //Get the WireToBlock for each block and check that no wire in blocks[i]'WireToBlock is
        //after any wire in blocks[i+1]'s WireToBlock:
        for (int i = 0; i < blocks.size() - 1; ++i) {
            auto&wire_to_block1 = registry.get<WiresToBlock>(blocks[i]);
            auto&wire_to_block2 = registry.get<WiresToBlock>(blocks[i + 1]);
            for (auto wire_id1: wire_to_block1.input_wire_ids) {
                if (wire_id1 == entt::null) {
                    continue;
                }
                for (auto wire_id2: wire_to_block2.input_wire_ids) {
                    if (wire_id2 == entt::null) {
                        continue;
                    }
                    REQUIRE(std::find(wires.begin(), wires.end(), wire_id1) <
                        std::find(wires.begin(), wires.end(), wire_id2));
                }
            }
        }
    }

    SECTION("Testing Blocks with Multiple Inputs and Outputs") {
        engine.add_wire(block1, block3, getOutputId(registry, block1, 0),
                        getInputId(registry, block3, 0), Wire::transmit_1d_to_1d);
        engine.add_wire(block2, block3, getOutputId(registry, block2, 0),
                        getInputId(registry, block3, 1), Wire::transmit_1d_to_1d);

        registry.get<Input1D>(registry.get<Block>(block1).inputIds[0]).value = 3.0f;
        registry.get<Input1D>(registry.get<Block>(block2).inputIds[0]).value = 2.0f;
        graph.process(ctx);

        auto&output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto&output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);
        auto&output3_0 = registry.get<Output1D>(registry.get<Block>(block3).outputIds[0]);
        auto&output3_1 = registry.get<Output1D>(registry.get<Block>(block3).outputIds[1]);

        REQUIRE(output1.value == 6.0f);
        REQUIRE(output2.value == 5.0f);
        REQUIRE(output3_0.value == 12.0f);
        REQUIRE(output3_1.value == 9.0f);
    }
    SECTION("Testing that cycles throw an error") {
        //Connect block1 -> block3 -> block2 -> block1
        engine.add_wire(block1, block3, getOutputId(registry, block1, 0),
                        getInputId(registry, block3, 0), Wire::transmit_1d_to_1d);
        engine.add_wire(block3, block2, getOutputId(registry, block3, 0),
                        getInputId(registry, block2, 0), Wire::transmit_1d_to_1d);
        REQUIRE_THROWS(engine.add_wire(block2, block1, getOutputId(registry, block2, 0),
            getInputId(registry, block1, 0), Wire::transmit_1d_to_1d));
    }

    SECTION("Testing wire with width > 1") {
        auto block4 = create_times_2_and_plus_4_vectorized(registry);
        engine.add_wire(block1, block4, getOutputId(registry, block1, 0),
                        getInputId(registry, block4, 0), Wire::broadcast_1d_to_2d);

        //Wiring in other blocks but we're not gonna check those
        engine.add_wire(block1, block3, getOutputId(registry, block1, 0),
                        getInputId(registry, block3, 0), Wire::transmit_1d_to_1d);
        engine.add_wire(block2, block3, getOutputId(registry, block2, 0),
                        getInputId(registry, block3, 1), Wire::transmit_1d_to_1d);

        registry.get<Input1D>(getInputId(registry, block1, 0)).value = 3.0f;
        graph.process(ctx);

        auto&output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto&output4 = registry.get<Output2D>(registry.get<Block>(block4).outputIds[0]);

        REQUIRE(output1.value == 6.0f);
        REQUIRE(output4.value[0] == 12.0f);
        REQUIRE(output4.value[1] == 10.0f);


        //Delete block 2 and 3 and check we still get the right output
        engine.remove_block(block2);
        engine.remove_block(block3);
        graph.process(ctx);
        REQUIRE(output1.value == 6.0f);
        REQUIRE(output4.value[0] == 12.0f);
        REQUIRE(output4.value[1] == 10.0f);
    }
}

TEST_CASE("Test mixers") {
    AudioEngine engine;
    auto [registry, guard] = engine.get_graph_registry();
    auto block1 = create_times_two(registry);
    auto block2 = create_plus_three(registry);
    auto block3 = create_times_2_and_plus_4(registry);
    auto block4 = create_times_2_and_plus_4_vectorized(registry);

    AudioContext ctx{48000.0f, 1.0f / 48000.0f, 0.5};
    auto&graph = engine._test_only_get_graph();

    SECTION("Test mono mixer") {
        auto mixer = MonoMixer<4>::create(&engine);
        engine.add_wire_to_mixer(block1, mixer, getOutputId(registry, block1, 0), 0,
                                 Wire::transmit_to_mono_mixer<4>);
        engine.add_wire_to_mixer(block2, mixer, getOutputId(registry, block2, 0), 1,
                                 Wire::transmit_to_mono_mixer<4>);

        registry.get<Input1D>(registry.get<Block>(block1).inputIds[0]).value = 3.0f;
        registry.get<Input1D>(registry.get<Block>(block2).inputIds[0]).value = 2.0f;
        graph.process(ctx);

        auto&output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto&output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);
        auto&output3 = registry.get<Output1D>(registry.get<Block>(mixer).outputIds[0]);

        REQUIRE(output1.value == 6.0f);
        REQUIRE(output2.value == 5.0f);
        REQUIRE(output3.value == 11.0f);
    }
}


int main(int argc, char* argv[]) {
    Catch::Session session; // There must be exactly one instance

    // writing to session.configData() here sets defaults
    // this is the preferred way to set them

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0) // Indicates a command line error
        return returnCode;

    // writing to session.configData() or session.Config() here
    // overrides command line args
    // only do this if you know you need to

    int numFailed = session.run();

    // numFailed is clamped to 255 as some unices only use the lower 8 bits.
    // This clamping has already been applied, so just return it here
    // You can also do any post run clean-up here
    return numFailed;
}
