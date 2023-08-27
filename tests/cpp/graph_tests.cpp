#define CATCH_CONFIG_ENABLE_BENCHMARKING
#define CATCH_CONFIG_RUNNER

#include <catch2/catch_all.hpp>
#include <../../src/core/graph.h>
#include <../../src/core/graph_io.h>

using namespace Graph;

// Define a couple of dummy blocks for testing purposes
class DummyBlock1 : public Block {
public:

    DummyBlock1() {}

    IMPLEMENT_BLOCK_IO(1, 1);

    virtual void process(AudioContext ctx) override {
        outputs()[0] = inputs()[0] * 2.0f;
    }

    virtual std::string name() override {
        return "DummyBlock1";
    }
};

class DummyBlock2 : public Block {
public:
    DummyBlock2() {}

    IMPLEMENT_BLOCK_IO(1, 1);


    virtual void process(AudioContext ctx) override {
        outputs()[0] = inputs()[0] + 3.0f;
    }

    virtual std::string name() override {
        return "DummyBlock2";
    }

};

// Unit tests
TEST_CASE("Testing AudioGraph with Dummy Blocks", "[AudioGraph]") {
    AudioGraph graph;

    auto _block1 = std::make_shared<DummyBlock1>();
    auto _block2 = std::make_shared<DummyBlock2>();

    DummyBlock1 *block1 = _block1.get();
    DummyBlock2 *block2 = _block2.get();

    graph.add_block(std::move(_block1));
    graph.add_block(std::move(_block2));

    SECTION("Adding and processing blocks") {
        AudioContext ctx = {44100.0f, 0.0};

// Simple processing test
        block1->outputs()[0] = 2.0f; // Set a value to block1's output
        graph.process(ctx);

        REQUIRE(block1->outputs()[0] == 2.0f); // block1 simply multiplies by 2
        REQUIRE(block2->outputs()[0] == 0.0f); // block2 hasn't been connected to anything so it should be 0

// Connecting the blocks and processing again
        graph.connect_wire(block1, block2, 0, 1, 0);
        graph.process(ctx);

        REQUIRE(block2->outputs()[0] == 7.0f); // block1 output is 2, so block2 output should be 2*2 + 3 = 7
    }

    SECTION("Testing disconnection") {
        graph.connect_wire(block1, block2, 0, 1, 0);

// Disconnect blocks using id
        size_t n;
        graph.disconnect_wire(block2->get_input_wires(n)[0].id, std::nullopt);

        AudioContext ctx = {44100.0f, 0.0};
        block1->outputs()[0] = 3.0f; // Set a value to block1's output
        graph.process(ctx);

        REQUIRE(block2->outputs()[0] == 0.0f); // block2 hasn't been connected to anything so it should be 0
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
