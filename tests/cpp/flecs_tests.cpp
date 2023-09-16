//
//
#define CATCH_CONFIG_MAIN // This tells Catch to provide a main() - only do this in one cpp file
#define CATCH_CONFIG_ENABLE_BENCHMARKING

#include <flecs.h>
#include <catch2/catch_all.hpp>


TEST_CASE("Test flecs graph", "[flecs]") {
    flecs::world ecs;

    // Define a few ids, a few components, pairs between ids and components:
    auto id1 = ecs.entity();
    auto id2 = ecs.entity();
    auto id3 = ecs.entity();

    auto comp1 = ecs.component<int>();
    auto comp2 = ecs.component<float>();
    auto comp3 = ecs.component<double>();

    id1.set(comp1, 1);
    id2.set(comp2, 2.0f);
    id3.set(comp3, 3.0);


}
