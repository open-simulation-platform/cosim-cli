cmake_minimum_required(VERSION 3.9)
project("cse-cli" VERSION "0.2.0")
set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/dist")

# ==============================================================================
# Global internal configuration
# ==============================================================================

# Use the highest warning levels and treat all warnings as errors,
# but ignore a few selected warnings.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options("-Wall" "-Wextra" "-Wpedantic" "-Werror")
    add_compile_options("-Wno-parentheses")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/W4" "/WX")
    add_compile_options("/wd4996" "/wd4251")
    add_definitions("-D_SCL_SECURE_NO_WARNINGS" "-D_CRT_SECURE_NO_WARNINGS")
endif()

if(APPLE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
else()
    set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
endif()

# ==============================================================================
# Dependencies
# ==============================================================================

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(NO_OUTPUT_DIRS)
else()
    message(FATAL_ERROR "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first!")
endif()

find_package(csecore REQUIRED)
find_package(Boost REQUIRED COMPONENTS program_options)

# ==============================================================================
# Targets
# ==============================================================================

set(generatedFilesDir "${CMAKE_BINARY_DIR}/generated")
file(WRITE "${generatedFilesDir}/project_version_from_cmake.hpp"
    "constexpr const char* project_version = \"${PROJECT_VERSION}\";\n")

add_executable(cse
    "src/cache.hpp"
    "src/cache.cpp"
    "src/clean_cache.hpp"
    "src/clean_cache.cpp"
    "src/cli_application.hpp"
    "src/cli_application.cpp"
    "src/console_utils.hpp"
    "src/console_utils.cpp"
    "src/inspect.hpp"
    "src/inspect.cpp"
    "src/logging_options.hpp"
    "src/logging_options.cpp"
    "src/main.cpp"
    "src/run.hpp"
    "src/run.cpp"
    "src/run_common.hpp"
    "src/run_common.cpp"
    "src/run_single.hpp"
    "src/run_single.cpp"
    "src/tools.hpp"
    "src/tools.cpp"
    "src/version_option.hpp"
    "src/version_option.cpp"
)
target_include_directories(cse PRIVATE "${generatedFilesDir}")
target_link_libraries(cse
    PRIVATE
        csecore::csecorecpp
        Boost::program_options)

# ==============================================================================
# Installation
# ==============================================================================

install(TARGETS cse RUNTIME DESTINATION bin)
include(InstallRequiredSystemLibraries)
