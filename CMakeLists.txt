cmake_minimum_required(VERSION 2.8)
project(SimpleFTP)

set(CMAKE_BUILD_TYPE "Debug")

# Settings and stuff
include(cmake/options.cmake)
include(cmake/settings.cmake)

# Display selected settings
include(cmake/showoptions.cmake)

add_subdirectory(src)

