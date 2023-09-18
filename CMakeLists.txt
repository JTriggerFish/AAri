cmake_minimum_required(VERSION 3.14)

project(AAri VERSION 0.0.1 LANGUAGES CXX)
add_definitions(-DNOMINMAX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Fetch SDL2 source code and set it up for building


# Fetch pybind11 source code and set it up for building
FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.11
)
FetchContent_MakeAvailable(pybind11)

# Find Python3 installation
find_package(Python3 COMPONENTS Development REQUIRED)

# Fetch Catch2 for tests
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.0.0-preview3
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
        flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
        GIT_TAG v3.2.7
)

FetchContent_MakeAvailable(flecs)
FetchContent_GetProperties(flecs)
if (NOT flecs_POPULATED)
    FetchContent_Populate(flecs)

    # Set the option to build Flecs as a static library
    set(FLECS_STATIC_LIBS ON CACHE BOOL "Build flecs as a static library" FORCE)

    add_subdirectory(${flecs_SOURCE_DIR} ${flecs_BINARY_DIR})
endif ()

# Define the core library
add_library(core STATIC
        src/core/utils/FieldMap.cpp
        src/core/wire.cpp
        src/core/block.cpp
        src/core/graph.cpp
        src/core/audio_engine.cpp
        src/blocks/oscillators.cpp
        src/blocks/envelopes.cpp
        src/blocks/mixers.cpp
)

# Get SDL2 include directories and link libraries
target_link_libraries(core PRIVATE Python3::Python pybind11::pybind11 flecs_static)

# Define the main executable
add_executable(AAri src/main.cpp)
target_link_libraries(AAri PRIVATE core)

# Define tests (we will use Catch2 for this)
enable_testing()
add_executable(utils_tests tests/cpp/utils_tests.cpp)
target_link_libraries(utils_tests PRIVATE core Catch2::Catch2WithMain pybind11::pybind11)

add_executable(graph_tests tests/cpp/graph_tests.cpp)
target_link_libraries(graph_tests PRIVATE core Catch2::Catch2WithMain pybind11::pybind11)

add_executable(flecs_tests tests/cpp/flecs_tests.cpp)
target_link_libraries(flecs_tests PRIVATE core Catch2::Catch2WithMain pybind11::pybind11 flecs_static)

# Define Python extension
pybind11_add_module(AAri_cpp bindings/python_bindings.cpp)
target_link_libraries(AAri_cpp PRIVATE core)

set_target_properties(AAri_cpp PROPERTIES LIBRARY_OUTPUT_DIRECTORY "../AAri")

# TODO Fis this , it is broken
#add_custom_command(TARGET AAri_cpp POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#        $<TARGET_FILE:AAri_cpp>
#        ../../tests/python)

# Install rules
install(TARGETS AAri RUNTIME DESTINATION bin)

# Set up tests
add_test(NAME UtilsTests COMMAND utils_tests)
add_test(NAME GraphTests COMMAND graph_tests)