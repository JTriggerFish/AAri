project('AAri', 'cpp', version:'0.0.1')

dependencies = [
    dependency('pybind11'),
    dependency('SDL2')
]

core_lib = static_library('core',
  ['src/core/utils/FieldMap.cpp',
   'src/core/graph.cpp',
   'src/core/audio_engine.cpp'],
)


executable('AAri', 'src/main.cpp', dependencies: dependencies, link_with: core_lib)


# Tests
test_deps = [
    dependency('catch2')
]
utils_tests = executable('utils_tests',  ['tests/cpp/utils_tests.cpp'], dependencies: test_deps, link_with:core_lib)
graph_tests = executable('graph_tests',  [ 'tests/cpp/graph_tests.cpp'], dependencies: test_deps, link_with:core_lib)

test('Utils tests', utils_tests)
test('Graph tests', graph_tests)
