cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
project(memory-tracer)

add_library(memory-tracer SHARED
  memory-tracer.c
  histogram_wrapper.cc
  histogram_wrapper.h
  histogram_approx.hpp
)
target_link_libraries(memory-tracer dl)
set_target_properties(
  memory-tracer
  PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)

