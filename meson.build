project('AAri', 'cpp')

dependencies = [
    dependency('pybind11'),
    dependency('SDL2')
]

source_files = files('src/main.cpp',
    'src/core/utils/FieldMap.cpp'
    )

executable('AAri', source_files, dependencies: dependencies)


# Tests
test_deps = [
    dependency('catch2')
]
catch_test_prog = executable('utils_tests', 'tests/cpp/utils_tests.cpp', dependencies: test_deps)
test('Utils tests', catch_test_prog)