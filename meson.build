project('AAri', 'cpp', version:'0.0.1', default_options: ['cpp_std=c++20'])
pymod = import('python')
py = pymod.find_installation('python3')

dependencies_core = [
    dependency('pybind11'),
    dependency('python3'),
    dependency('SDL2')
]
dependencies_python = [
    dependency('pybind11'),
    dependency('python3'),
]

core_files = ['src/core/utils/FieldMap.cpp',
   'src/core/graph.cpp',
   'src/core/audio_engine.cpp',
   'src/blocks/oscillators.cpp',
   'src/blocks/mixers.cpp',
   ]

core_lib = static_library('core', core_files, dependencies: dependencies_core)

executable('AAri', 'src/main.cpp', dependencies: dependencies_core, link_with: core_lib)

py.extension_module('AAri_cpp', ['bindings/python_bindings.cpp'],
 dependencies: dependencies_core + dependencies_python,
  link_with: core_lib,
   # install_dir: py.get_install_dir())
   install_dir: 'tests/python')


# Tests
test_deps = [
    dependency('catch2')
] + dependencies_core

utils_tests = executable('utils_tests',  ['tests/cpp/utils_tests.cpp'], dependencies: test_deps, link_with:core_lib)
graph_tests = executable('graph_tests',  [ 'tests/cpp/graph_tests.cpp'], dependencies: test_deps, link_with:core_lib)

test('Utils tests', utils_tests)
test('Graph tests', graph_tests)