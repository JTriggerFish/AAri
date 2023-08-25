project('AAri', 'cpp')

pybind11_dep = dependency('pybind11')
sdl2_dep = dependency('SDL2')

source_files = files('src/main.cpp',
    'src/core/utils/FieldMap.cpp')
executable('AAri', source_files, dependencies: [pybind11_dep, sdl2_dep])

catch2_dep = dependency('catch2')
catch_test_prog = executable('utils_tests', 'tests/cpp/utils_tests.cpp', dependencies:[catch2_dep])
test('Utils tests', catch_test_prog)