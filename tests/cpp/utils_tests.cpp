#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include "../../src/core/inputs_outputs.h"
#include "../../src/core/graph.h"
#include "../../src/core/audio_engine.h"
#include "../../src/blocks/mixers.h"
#include <entt/entt.hpp>
#include <catch2/catch_all.hpp>

TEST_CASE("Test fill with nulls")
{
    SECTION("Test correct size and actually null")
    {
        std::array<entt::entity, 10> arr = fill_with_null<10>();
        REQUIRE(arr.size() == 10);
        for (auto &item: arr)
            REQUIRE((item == entt::null));
    }
    std::array<entt::entity, 10> arr = fill_with_null<10>();
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