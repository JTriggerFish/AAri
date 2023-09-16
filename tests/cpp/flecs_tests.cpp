//
//
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <flecs.h>
#include <catch2/catch_all.hpp>

struct Frequency {
    float value;
};
struct Amplitude {
    float value;
};
struct Phase {
    float value;
};
struct Output {
    float value;
};

TEST_CASE("Test flecs graph") {
    flecs::world ecs;

    SECTION("Testing can define wires") {
        // Define a few ids, a few components, pairs between ids and components:
        auto Osc = ecs.entity();
        auto Output = ecs.entity();
        auto Wire = ecs.entity();

        //Attach freq to osc:
        Osc.set<Frequency>({440.0f});
        // Create graph edge from osc to output entity
        Output.add(Wire, Osc);

        REQUIRE(ecs_has_pair(ecs.c_ptr(), Output.id(), Wire.id(), Osc.id()));

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
