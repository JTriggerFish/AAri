#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include "../../src/core/parameters.h"
#include "../../src/core/graph.h"
#include "../../src/core/blocks.h"
#include "../../src/core/audio_engine.h"
#include <entt/entt.hpp>
#include <catch2/catch_all.hpp>

using namespace AAri;

void times_two(entt::registry &registry, const Block &block, AudioContext ctx) {
    auto &input = registry.get<Input1D>(block.inputIds[0]);
    auto &output = registry.get<Output1D>(block.outputIds[0]);
    output.value = input.value * 2.0f;
}

void plus_three(entt::registry &registry, const Block &block, AudioContext ctx) {
    auto &input = registry.get<Input1D>(block.inputIds[0]);
    auto &output = registry.get<Output1D>(block.outputIds[0]);
    output.value = input.value + 3.0f;
}

void times_2_and_plus_4(entt::registry &registry, const Block &block, AudioContext ctx) {
    auto &input = registry.get<Input2D>(block.inputIds[0]);
    auto &output = registry.get<Output2D>(block.outputIds[0]);
    output.value[0] = input.value[0] * 2.0f;
    output.value[1] = input.value[1] + 4.0f;
}

entt::entity create_times_two(entt::registry &registry) {
    auto input = registry.create();
    registry.emplace<Input1D>(input, 0.0f, ParamName::Input);
    auto output = registry.create();
    registry.emplace<Output1D>(output, 0.0f, ParamName::Out);

    return Block::create(registry, BlockType::Product, std::array<entt::entity, 8>{input},
                         std::array<entt::entity, 4>{output}, times_two);
}

entt::entity create_plus_three(entt::registry &registry) {
    auto input = registry.create();
    registry.emplace<Input1D>(input, 0.0f, ParamName::Input);
    auto output = registry.create();
    registry.emplace<Output1D>(output, 0.0f, ParamName::Out);

    return Block::create(registry, BlockType::Sum, std::array<entt::entity, 8>{input},
                         std::array<entt::entity, 4>{output}, plus_three);
}


TEST_CASE("Testing audio start stop with graph callback", "[AudioEngine]") {
    AudioEngine engine;

    SECTION("Testing start and stop") {
        engine.startAudio();
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
    auto &graph = engine._test_only_get_graph();


    SECTION("Adding and processing blocks") {
        //Set the input of block 2 to 1 :
        registry.get<Input1D>(registry.get<Block>(block2).inputIds[0]).value = 1.0f;

        //Process one sample on the graph
        graph.process(ctx);
        //Get the output of the blocks
        auto &output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto &output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);

        REQUIRE(output1.value == 3.0f);
        REQUIRE(output2.value == 2.0f);
    }

        // Note that each section rebuilds the graph, so the ids will be different
    SECTION("Testing connection") {
        //Connecting the blocks and processing again
        auto &output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto &output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);

        // Set block 2 input to 2
        registry.get<Input1D>(registry.get<Block>(block2).inputIds[0]).value = 2.0f;

        auto wire = engine.add_wire(block2, block1, registry.get<Block>(block2).outputIds[0],
                                    registry.get<Block>(block1).inputIds[0], Wire::transmit_1d_to_1d);

        //Process one sample on the graph
        graph.process(ctx);

        REQUIRE(output2.value == 4.0f);
        REQUIRE(output1.value == 7.0f);

        //Adding the same wire again should throw an exception
        REQUIRE_THROWS(engine.add_wire(block2, block1, registry.get<Block>(block2).outputIds[0],
                                       registry.get<Block>(block1).inputIds[0], Wire::transmit_1d_to_1d));
    }

    SECTION("Testing disconnection") {
        engine.add_wire(block2, block1, registry.get<Block>(block2).outputIds[0],
                        registry.get<Block>(block1).inputIds[0], Wire::transmit_1d_to_1d);

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
        auto &output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto &output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);

        // Back to disconnected expected values
        REQUIRE(output1.value == 3.0f);
        REQUIRE(output2.value == 2.0f);

    }SECTION("Testing wire gain and offset") {
        auto wire = engine.add_wire(block2, block1, registry.get<Block>(block2).outputIds[0],
                                    registry.get<Block>(block1).inputIds[0], Wire::transmit_1d_to_1d);

        // Check gain and offset are as expected:
        REQUIRE(engine.get_wire(wire).gain == 1.0f);
        REQUIRE(engine.get_wire(wire).offset == 0.0f);

        // Set wire gain to zero:
        engine.tweak_wire_gain(wire, 0.0f);
        REQUIRE(engine.get_wire(wire).gain == 0.0f);
        REQUIRE(engine.get_wire(wire).offset == 0.0f);

        //Check offset tweaking then set it back
        engine.tweak_wire_offset(wire, 1.0f);
        REQUIRE(engine.get_wire(wire).gain == 0.0f);
        REQUIRE(engine.get_wire(wire).offset == 1.0f);
        engine.tweak_wire_offset(wire, 0.0f);

        //Set the input of block 2 to 1 :
        registry.get<Input1D>(registry.get<Block>(block2).inputIds[0]).value = 1.0f;
        //Process one sample on the graph
        graph.process(ctx);
        //Get the output of the blocks
        auto &output1 = registry.get<Output1D>(registry.get<Block>(block1).outputIds[0]);
        auto &output2 = registry.get<Output1D>(registry.get<Block>(block2).outputIds[0]);

        // Back to disconnected expected values
        REQUIRE(output1.value == 3.0f);
        REQUIRE(output2.value == 2.0f);

    }
}


TEST_CASE("Additional Testing of AudioGraph with Multiple Scenarios", "[AudioGraph]") {


    SECTION("Testing Multiple layers of dependencies") {
    }

    SECTION("Testing Blocks with Multiple Inputs and Outputs") {
    }SECTION("Testing that cycles throw an error") {

    }

    SECTION("Testing wire with width > 1") {
    }

    SECTION("Testing Overlapping Wire Regions") {
    }
}


int main(int argc, char *argv[]) {
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
