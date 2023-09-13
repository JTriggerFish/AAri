#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_RUNNER

#include <catch2/catch_all.hpp>
#include <../src/core/graph.h>
#include <../src/core/graph_io.h>

using namespace Graph;

// Define a couple of dummy blocks for testing purposes
class DummyBlock1 : public Block {
public:

    DummyBlock1() {}

    IMPLEMENT_BLOCK_IO(1, 1);

    virtual void process(AudioContext ctx) override {
        io.outputs[0] = io.inputs[0] * 2.0f;
    }

    virtual std::string name() const override {
        return "DummyBlock1_" + std::to_string(id());
    }
};

class DummyBlock2 : public Block {
public:
    DummyBlock2() {}

    IMPLEMENT_BLOCK_IO(1, 1);


    virtual void process(AudioContext ctx) override {
        io.outputs[0] = io.inputs[0] + 3.0f;
    }

    virtual std::string name() const override {
        return "DummyBlock2_" + std::to_string(id());
    }

};

// Unit tests
TEST_CASE("Testing AudioGraph with Dummy Blocks", "[AudioGraph]") {
    AudioGraph graph(-1);

    auto block1 = std::make_shared<DummyBlock2>();
    auto block2 = std::make_shared<DummyBlock1>();

    // Cheat a bit and use const_cast to get around the constness of the returned pointer
    graph.add_block(block1);
    graph.add_block(block2);

    SECTION("Adding and processing blocks") {
        AudioContext ctx = {44100.0f, 0.1f};

        // Simple processing test
        block2->inputs()[0] = 1.0f;
        graph.process(ctx);

        REQUIRE(block1->last_processed_time == 0.1f);
        REQUIRE(block2->last_processed_time == 0.1f);

        REQUIRE(block2->outputs()[0] == 2.0f); // DummyBlock1 simply multiplies by 2
        REQUIRE(block1->outputs()[0] == 3.0f); // DummyBlock2 adds 3
    }

        // Note that each section rebuilds the graph, so the ids will be different
    SECTION("Testing connection") {
        // Connecting the blocks and processing again
        graph.connect_wire(block2->id(), block1->id(), 0, 1, 0);
        AudioContext ctx = {44100.0f, 0.2f};
        block2->inputs()[0] = 2.0f;
        graph.process(ctx);

        REQUIRE(block1->last_processed_time == 0.2f);
        REQUIRE(block2->last_processed_time == 0.2f);
        REQUIRE(block2->outputs()[0] == 4.0f); // DummyBlock1 simply multiplies by 2
        REQUIRE(block1->outputs()[0] == 7.0f); // block2 output is 2, so block1 output should be 2*2+ 3 = 7

        //Adding the wire again should throw an exception
        REQUIRE_THROWS(graph.connect_wire(block2->id(), block1->id(), 0, 1, 0));
    }

    SECTION("Testing disconnection") {
        graph.connect_wire(block2->id(), block1->id(), 0, 1, 0);
        block2->inputs()[0] = 1.0f;

// Disconnect blocks using id
        size_t n;
        // There is no wire connected to block2
        REQUIRE_THROWS(graph.disconnect_wire(block2->get_input_wires(n)[0].id));

        graph.disconnect_wire(block1->get_input_wires(n)[0].id);

        AudioContext ctx = {44100.0f, 0.3f};
        graph.process(ctx);
        REQUIRE(block1->last_processed_time == 0.3f);
        REQUIRE(block2->last_processed_time == 0.3f);

        REQUIRE(block2->outputs()[0] == 2.0f); // block1 simply multiplies by 2
        REQUIRE(block1->outputs()[0] == 3.0f); // should be back to initial output
    }
}


// Define a new dummy block with multiple inputs and outputs
class DummyBlock3 : public Block {
public:
    DummyBlock3() {}

    IMPLEMENT_BLOCK_IO(2, 2);

    virtual void process(AudioContext ctx) override {
        io.outputs[0] = io.inputs[0] * 2.0f;
        io.outputs[1] = io.inputs[1] + 4.0f;
    }

    virtual std::string name() const override {
        return "DummyBlock3_" + std::to_string(id());
    }
};

TEST_CASE("Additional Testing of AudioGraph with Multiple Scenarios", "[AudioGraph]") {
    AudioGraph graph(-1);

    auto block1 = std::make_shared<DummyBlock1>();
    auto block2 = std::make_shared<DummyBlock2>();
    auto block3 = std::make_shared<DummyBlock3>();

    graph.add_block(block1);
    graph.add_block(block2);
    graph.add_block(block3);


    SECTION("Testing Multiple layers of dependencies") {
        // Connect block1 -> block3 -> block2
        graph.connect_wire(block1->id(), block3->id(), 0, 1, 0);
        graph.connect_wire(block3->id(), block2->id(), 0, 1, 0);

        AudioContext ctx = {44100.0f, 0.5f};
        block1->inputs()[0] = 2.0f;
        graph.process(ctx);

        REQUIRE(block1->outputs()[0] == 4.0f);
        REQUIRE(block3->outputs()[0] == 8.0f);
        REQUIRE(block2->outputs()[0] == 11.0f);
    }

    SECTION("Testing Blocks with Multiple Inputs and Outputs") {
        graph.connect_wire(block1->id(), block3->id(), 0, 1, 0);
        graph.connect_wire(block2->id(), block3->id(), 0, 1, 1);

        AudioContext ctx = {44100.0f, 0.6f};
        block1->inputs()[0] = 3.0f;
        block2->inputs()[0] = 2.0f;
        graph.process(ctx);

        REQUIRE(block1->outputs()[0] == 6.0f);
        REQUIRE(block2->outputs()[0] == 5.0f);
        REQUIRE(block3->outputs()[0] == 12.0f);
        REQUIRE(block3->outputs()[1] == 9.0f);
    }SECTION("Testing that cycles throw an error") {
        // Connect block1 -> block3 -> block2 -> block1
        graph.connect_wire(block1->id(), block3->id(), 0, 1, 0);
        graph.connect_wire(block3->id(), block2->id(), 0, 1, 0);
        REQUIRE_THROWS(graph.connect_wire(block2->id(), block1->id(), 0, 1, 0));

    }

    SECTION("Testing wire with width > 1") {
        auto block4 = std::make_shared<DummyBlock3>();
        graph.add_block(block4);

        graph.connect_wire(block3->id(), block4->id(), 0, 2, 0);

        AudioContext ctx = {44100.0f, 0.7f};
        block3->inputs()[0] = 1.0f;
        block3->inputs()[1] = 1.0f;

        graph.process(ctx);
        REQUIRE(block3->outputs()[0] == 2.0f);
        REQUIRE(block3->outputs()[1] == 5.0f);
        REQUIRE(block4->outputs()[0] == 4.0f);
        REQUIRE(block4->outputs()[1] == 9.0f);

        // This should throw because block 1 output is of size 1
        REQUIRE_THROWS(graph.connect_wire(block1->id(), block3->id(), 0, 2, 0));
        //This should throw because block 2 input is of size 1
        REQUIRE_THROWS(graph.connect_wire(block3->id(), block2->id(), 0, 2, 0));
    }

    SECTION("Testing Overlapping Wire Regions") {
        auto block4 = std::make_shared<DummyBlock3>();
        graph.add_block(block4);

        graph.connect_wire(block3->id(), block4->id(), 0, 2, 0);
        REQUIRE_THROWS(graph.connect_wire(block1->id(), block4->id(), 0, 1, 1));  // This should throw due to overlap
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
