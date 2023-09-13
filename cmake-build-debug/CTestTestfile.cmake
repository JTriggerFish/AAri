# CMake generated Testfile for 
# Source directory: D:/Dev/AAri
# Build directory: D:/Dev/AAri/cmake-build-debug
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(UtilsTests "D:/Dev/AAri/cmake-build-debug/utils_tests.exe")
set_tests_properties(UtilsTests PROPERTIES  _BACKTRACE_TRIPLES "D:/Dev/AAri/CMakeLists.txt;73;add_test;D:/Dev/AAri/CMakeLists.txt;0;")
add_test(GraphTests "D:/Dev/AAri/cmake-build-debug/graph_tests.exe")
set_tests_properties(GraphTests PROPERTIES  _BACKTRACE_TRIPLES "D:/Dev/AAri/CMakeLists.txt;74;add_test;D:/Dev/AAri/CMakeLists.txt;0;")
subdirs("_deps/sdl2-build")
subdirs("_deps/pybind11-build")
subdirs("_deps/catch2-build")
