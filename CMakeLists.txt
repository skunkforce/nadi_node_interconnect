cmake_minimum_required(VERSION 3.27)

set(PROJECT_NAME "nadi_interconnect")

# Set C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # Disable compiler-specific extensions

include(FetchContent)


if(NOT TARGET CLI11::CLI11)
    set(CLI11_BUILD_TESTS OFF)
    set(CLI11_BUILD_EXAMPLES OFF)
    # CLI11
    FetchContent_Declare(
        cli11_proj
        QUIET
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.6.1
    )

    FetchContent_MakeAvailable(cli11_proj)
endif()

if(NOT TARGET nlohmann_json::nlohmann_json)
    FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
    set(JSON_BuildTests OFF CACHE INTERNAL "")
    FetchContent_MakeAvailable(json)
endif()

if(NOT TARGET nadicpp::nadicpp)
    add_subdirectory(nadicpp)
endif()


# Get latest git tag for version
execute_process(
    COMMAND git describe --tags --abbrev=0
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Fallback if no git tag exists
if(NOT GIT_VERSION)
    set(GIT_VERSION "0.0.0")
endif()

message(STATUS "Project version: ${GIT_VERSION}")
add_definitions(-DPROJECT_VERSION="${GIT_VERSION}")

project(${PROJECT_NAME} VERSION 0.0.1)

if(MSVC)
    add_compile_options(
        /EHsc  # Enable exceptions
        /GR    # Enable RTTI
        $<$<CONFIG:>:/MD>
        $<$<CONFIG:Debug>:/MTd>
        $<$<CONFIG:Release>:/MT>
    )
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp src/thread.cpp src/threads.cpp
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        nlohmann_json::nlohmann_json
        CLI11::CLI11
        nadi::nadi
        nadicpp::nadicpp
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

# -------------------------------------------------
# Optional tests
# -------------------------------------------------
option(NADI_INTERCONNECT_BUILD_TESTS "Build tests" ON)
if(NADI_INTERCONNECT_BUILD_TESTS)
    add_library(${PROJECT_NAME}_test_generator SHARED tests/generator.cpp)
    target_link_libraries(${PROJECT_NAME}_test_generator
        PRIVATE
            nadi::nadi
            nadicpp::nadicpp
            nlohmann_json::nlohmann_json
    )

    add_library(${PROJECT_NAME}_test_printer SHARED tests/printer.cpp)
    target_link_libraries(${PROJECT_NAME}_test_printer
        PRIVATE
            nadi::nadi
            nadicpp::nadicpp
            nlohmann_json::nlohmann_json
    )
endif()