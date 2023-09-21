cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
project(memory-tracer C CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard to use")

add_library(memory-tracer SHARED
  histogram_wrapper.cc
  histogram_wrapper.h
  histogram_approx.hpp
  memory-tracer.c
)
target_link_libraries(memory-tracer dl c++ c++abi)

set_target_properties(
  memory-tracer
  PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

target_compile_features(
  memory-tracer PUBLIC
  cxx_std_17
)
