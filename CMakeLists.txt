cmake_minimum_required(VERSION 3.9)
project("cse-cli" VERSION "0.1.0")

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

# Enable CMake to find our custom modules.
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}/cmake")

# ==============================================================================
# Dependencies
# ==============================================================================

if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(NO_OUTPUT_DIRS)
else()
    message(FATAL_ERROR "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first!")
endif()

find_package(Boost REQUIRED COMPONENTS date_time fiber filesystem log program_options)
find_package(LIBZIP REQUIRED)
find_package(FMILibrary REQUIRED)
find_package(GSL REQUIRED)
find_package(csecore REQUIRED)

# ==============================================================================
# Targets
# ==============================================================================

add_executable(cse
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
    "src/version_option.hpp"
    "src/version_option.cpp"
)
target_link_libraries(cse
    PRIVATE
        csecore::csecorecpp
        Boost::program_options)
