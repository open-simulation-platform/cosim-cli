cmake_minimum_required(VERSION 3.15)
project("cosim" VERSION "0.8.0")

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/dist" CACHE PATH "Install prefix" FORCE)
endif()

# To enable verbose when needed
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Suppress boost warnings for using version 1.81.0 (may not be needed for future release of cmake)
set(Boost_NO_WARN_NEW_VERSIONS ON)

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

find_package(libcbor REQUIRED)
find_package(libcosim REQUIRED)
find_package(Boost REQUIRED COMPONENTS log program_options)

# ==============================================================================
# Targets
# ==============================================================================

set(generatedFilesDir "${CMAKE_BINARY_DIR}/generated")
file(WRITE "${generatedFilesDir}/project_version_from_cmake.hpp"
    "constexpr const char* project_version = \"${PROJECT_VERSION}\";\n")

add_executable(cosim
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
target_include_directories(cosim PRIVATE "${generatedFilesDir}")
target_link_libraries(cosim PRIVATE libcosim::cosim Boost::log Boost::program_options)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # This makes the linker set RPATH rather than RUNPATH for the resulting
    # binary, so that the indirect dependencies between dynamic libraries in
    # our installation directory resolve correctly. (RPATH is transitive,
    # RUNPATH is not.)
    target_link_options(cosim PRIVATE "LINKER:--disable-new-dtags")
endif()

# ==============================================================================
# Installation
# ==============================================================================

install(TARGETS cosim RUNTIME DESTINATION bin)
include(InstallRequiredSystemLibraries)
install(FILES "LICENSE" "README.md" DESTINATION "doc")
