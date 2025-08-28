cmake_minimum_required(VERSION 3.3)

project(lab)

# Optional program arguments, for example:
# set(VALIDATE_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/input.file" "${CMAKE_CURRENT_SOURCE_DIR}/output.file")
# set(LAB_ARGS "${CMAKE_CURRENT_SOURCE_DIR}/input.file" "output.file")

# Add option to enable software prefetching optimization
option(ENABLE_SOLUTION "Enable software prefetching optimization" OFF)

string(REGEX MATCH "^(.*)[\\/]labs[\\/].*$" repo "${CMAKE_CURRENT_SOURCE_DIR}")
include(${CMAKE_MATCH_1}/tools/labs.cmake)

# Add SOLUTION macro definition if enabled
if(ENABLE_SOLUTION)
    add_definitions(-DSOLUTION)
    message(STATUS "Software prefetching optimization enabled")
else()
    message(STATUS "Software prefetching optimization disabled")
endif()
