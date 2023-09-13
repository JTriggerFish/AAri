# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Dev/AAri/cmake-build-debug/_deps/pybind11-src"
  "D:/Dev/AAri/cmake-build-debug/_deps/pybind11-build"
  "D:/Dev/AAri/cmake-build-debug/_deps/pybind11-subbuild/pybind11-populate-prefix"
  "D:/Dev/AAri/cmake-build-debug/_deps/pybind11-subbuild/pybind11-populate-prefix/tmp"
  "D:/Dev/AAri/cmake-build-debug/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp"
  "D:/Dev/AAri/cmake-build-debug/_deps/pybind11-subbuild/pybind11-populate-prefix/src"
  "D:/Dev/AAri/cmake-build-debug/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Dev/AAri/cmake-build-debug/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Dev/AAri/cmake-build-debug/_deps/pybind11-subbuild/pybind11-populate-prefix/src/pybind11-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
