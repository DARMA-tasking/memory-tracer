cmake_minimum_required(VERSION 3.17 FATAL_ERROR)
project(memory-tracer)

add_library(memory-tracer SHARED memory-tracer.c)
set_target_properties(
  memory-tracer
  PROPERTIES
  POSITION_INDEPENDENT_CODE ON
)
