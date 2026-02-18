# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly

if(NOT BUILD_TYPE)
    set(BUILD_TYPE debug)
endif()

message(STATUS "Build type: ${BUILD_TYPE}")

if(BUILD_TYPE STREQUAL "release")
    add_compile_definitions(APP_RELEASE_BUILD=1)
else()
    add_compile_definitions(APP_DEBUG_BUILD=1)
endif()


cmake_minimum_required(VERSION 3.16)

# In this project, we use the IDF component storage, which is located in the components/storage directory. We need to tell CMake to include this directory when building our project.
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(day2_buildSystems)

# Freeze build timestamp (reproducible build)
add_compile_definitions(
    ESP_APP_DESC_BUILD_DATE="1970-01-01"
    ESP_APP_DESC_BUILD_TIME="00:00:00"
)


# To make the build reproducible, we need to ensure that the file paths in the debug info are consistent across builds. We can achieve this by using the -ffile-prefix-map option to map the source and build directories to a fixed prefix.
add_compile_options(
    -ffile-prefix-map=${CMAKE_SOURCE_DIR}=.
    -ffile-prefix-map=${CMAKE_BINARY_DIR}=.
)
# We can also set the project version using a CMake variable. This variable can be used in our code to print the version information.

set(PROJECT_VER "1.0.0")
