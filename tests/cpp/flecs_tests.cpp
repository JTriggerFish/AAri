//
//
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <flecs.h>
#include <catch2/catch_all.hpp>
#include <iostream>

struct Frequency {
    float value;
};
struct Out {
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
struct StereoOutput {
    float left_right[2];
};

struct NamedToNamed {
    flecs::entity from_component;
    flecs::entity to_component;
};
struct NamedToNamedNumbered {
    flecs::entity from_component;
    flecs::entity to_component;
    size_t to_component_index;
};

TEST_CASE("Test flecs graph") {
    flecs::world ecs;

    ecs.component<Frequency>();
    ecs.component<Out>();
    ecs.component<StereoOutput>();
    ecs.component<NamedToNamed>();
    ecs.component<NamedToNamedNumbered>();

    SECTION("Testing can define wires") {
        auto Osc1 = ecs.entity();
        auto Osc2 = ecs.entity();
        auto Output = ecs.entity();
        auto WireTo = ecs.entity();

        // Attach initial values to components
        Osc1.set<Frequency>({440.0f});
        Osc1.set<Out>({0.0f});
        Osc2.set<Frequency>({220.0f});
        Osc2.set<Out>({0.0f});
        Output.set<StereoOutput>({{0.0f, 0.0f}});

        Osc1.set<NamedToNamedNumbered>(Output, {ecs.entity<Out>(),
                                                ecs.entity<StereoOutput>(),
                                                0});

        // Check the relationships and associated mappings
        Osc1.each([](flecs::id id) {
            std::cout << id.str() << std::endl;
            if (id.is_pair()) {
                std::cout << id.str() << std::endl;
                flecs::entity first = id.first();
                flecs::entity second = id.second();
                std::cout << "Pair: " << first << " -> " << second << std::endl;
            }
        });

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
