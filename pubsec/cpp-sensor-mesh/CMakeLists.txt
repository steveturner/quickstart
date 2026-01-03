cmake_minimum_required(VERSION 3.14)

project(cuas-mesh
  VERSION 1.0
  LANGUAGES CXX
  DESCRIPTION "Ditto Counter-UAS Sensor Mesh"
)

option(DITTO_QUICKSTART_TUI "Enable FTXUI library support" ON)
option(BUILD_TESTS "Build tests" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
  add_compile_options(-Wno-deprecated-declarations)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wno-deprecated-declarations)
  add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address)
endif()

# Fetch dependencies
include(FetchContent)

# FTXUI for TUI
if(DITTO_QUICKSTART_TUI)
  FetchContent_Declare(ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
    GIT_TAG v5.0.0
  )
  FetchContent_MakeAvailable(ftxui)
endif()

# nlohmann/json for serialization
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/nlohmann/json
  GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# cxxopts for CLI parsing
FetchContent_Declare(cxxopts
  GIT_REPOSITORY https://github.com/jarro2783/cxxopts
  GIT_TAG v3.1.1
)
FetchContent_MakeAvailable(cxxopts)

# Google Test for testing
if(BUILD_TESTS)
  FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG v1.14.0
  )
  FetchContent_MakeAvailable(googletest)
endif()

# Generate env.h from .env
set(ENV_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/../../.env)
set(ENV_H ${CMAKE_CURRENT_SOURCE_DIR}/src/env.h)
set(ENV_SCRIPT ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_env.awk)
find_program(AWK NAMES awk nawk gawk)
add_custom_command(
  OUTPUT ${ENV_H}
  COMMAND ${AWK} -f ${ENV_SCRIPT} ${ENV_INPUT} > ${ENV_H}
  DEPENDS ${ENV_INPUT} ${ENV_SCRIPT}
  COMMENT "Generating env.h from .env"
)
add_custom_target(env_h DEPENDS ${ENV_H} ${ENV_SCRIPT})

# Collect source files
set(MODEL_SOURCES
  src/models/sensor.cpp
  src/models/detection.cpp
  src/models/track.cpp
  src/models/alert.cpp
  src/models/assignment.cpp
)

set(DITTO_SOURCES
  src/ditto/mesh_peer.cpp
)

set(NODE_SOURCES
  src/nodes/sensor_node.cpp
  src/nodes/c2_node.cpp
  src/nodes/effector_node.cpp
)

set(SIM_SOURCES
  src/simulation/drone_sim.cpp
  src/correlation/correlator.cpp
)

set(TUI_SOURCES
  src/tui/app.cpp
)

# Main executable
add_executable(cuas-mesh
  src/main.cpp
  ${MODEL_SOURCES}
  ${DITTO_SOURCES}
  ${NODE_SOURCES}
  ${SIM_SOURCES}
  ${TUI_SOURCES}
)
add_dependencies(cuas-mesh env_h)

# Demo executable (all nodes in single process)
add_executable(cuas-demo
  src/demo.cpp
  ${MODEL_SOURCES}
  ${DITTO_SOURCES}
  ${NODE_SOURCES}
  ${SIM_SOURCES}
)
add_dependencies(cuas-demo env_h)

# Data flow test (runs without gtest)
add_executable(test-data-flow
  tests/test_data_flow.cpp
  ${MODEL_SOURCES}
  ${DITTO_SOURCES}
  ${NODE_SOURCES}
  ${SIM_SOURCES}
)
target_include_directories(test-data-flow PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/sdk
)

target_include_directories(cuas-mesh PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/sdk
)

target_include_directories(cuas-demo PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/sdk
)

# Find Ditto SDK or use stub
# Priority: 1) DITTO_SDK_PATH, 2) sdk/ directory (from Makefile download), 3) stub
set(DITTO_SDK_PATH "" CACHE PATH "Path to Ditto SDK")

if(EXISTS "${DITTO_SDK_PATH}/libditto.a")
  # SDK path provided with libditto.a at root
  set(DITTO_LIB "${DITTO_SDK_PATH}/libditto.a")
  set(DITTO_INCLUDE "${DITTO_SDK_PATH}")
  set(DITTO_USING_REAL_SDK TRUE)
  message(STATUS "Using Ditto SDK from: ${DITTO_SDK_PATH}")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sdk/libditto.a")
  # SDK downloaded via Makefile to sdk/ directory
  set(DITTO_LIB "${CMAKE_CURRENT_SOURCE_DIR}/sdk/libditto.a")
  set(DITTO_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/sdk")
  set(DITTO_USING_REAL_SDK TRUE)
  message(STATUS "Using Ditto SDK from: ${CMAKE_CURRENT_SOURCE_DIR}/sdk")
else()
  # Fall back to stub implementation
  set(DITTO_LIB "")
  set(DITTO_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}/sdk/stub")
  set(DITTO_USING_REAL_SDK FALSE)
  message(STATUS "Using Ditto SDK stub (demo mode)")
  message(STATUS "  For real P2P sync, run: make download-sdk (Linux only)")
endif()

# Define compile flag if using stub
if(NOT DITTO_USING_REAL_SDK)
  add_compile_definitions(DITTO_STUB_MODE)
endif()

target_include_directories(cuas-mesh PRIVATE ${DITTO_INCLUDE})
target_include_directories(cuas-demo PRIVATE ${DITTO_INCLUDE})
target_include_directories(test-data-flow PRIVATE ${DITTO_INCLUDE})

# Link libraries
target_link_libraries(cuas-mesh PRIVATE
  nlohmann_json::nlohmann_json
  cxxopts::cxxopts
)
target_link_libraries(cuas-demo PRIVATE
  nlohmann_json::nlohmann_json
)
target_link_libraries(test-data-flow PRIVATE
  nlohmann_json::nlohmann_json
)
if(DITTO_LIB)
  target_link_libraries(cuas-mesh PRIVATE ${DITTO_LIB})
  target_link_libraries(cuas-demo PRIVATE ${DITTO_LIB})
  target_link_libraries(test-data-flow PRIVATE ${DITTO_LIB})
endif()

if(DITTO_QUICKSTART_TUI)
  target_compile_definitions(cuas-mesh PRIVATE DITTO_QUICKSTART_TUI)
  target_link_libraries(cuas-mesh PRIVATE
    ftxui::screen
    ftxui::dom
    ftxui::component
  )
endif()

# Tests
if(BUILD_TESTS)
  enable_testing()

  # Test models
  add_executable(test_models
    tests/test_models.cpp
    ${MODEL_SOURCES}
  )
  target_include_directories(test_models PRIVATE src ${DITTO_INCLUDE})
  target_link_libraries(test_models PRIVATE
    GTest::gtest_main
    nlohmann_json::nlohmann_json
  )
  add_test(NAME TestModels COMMAND test_models)

  # Test correlation
  add_executable(test_correlation
    tests/test_correlation.cpp
    ${MODEL_SOURCES}
    src/correlation/correlator.cpp
  )
  target_include_directories(test_correlation PRIVATE src ${DITTO_INCLUDE})
  target_link_libraries(test_correlation PRIVATE
    GTest::gtest_main
    nlohmann_json::nlohmann_json
  )
  add_test(NAME TestCorrelation COMMAND test_correlation)

  # Test Ditto sync (integration)
  add_executable(test_ditto_sync
    tests/test_ditto_sync.cpp
    ${MODEL_SOURCES}
  )
  target_include_directories(test_ditto_sync PRIVATE src ${DITTO_INCLUDE})
  target_link_libraries(test_ditto_sync PRIVATE
    GTest::gtest_main
    nlohmann_json::nlohmann_json
  )
  add_test(NAME TestDittoSync COMMAND test_ditto_sync)

  # Test workflow (integration)
  add_executable(test_workflow
    tests/test_workflow.cpp
    ${MODEL_SOURCES}
    src/correlation/correlator.cpp
    src/simulation/drone_sim.cpp
  )
  target_include_directories(test_workflow PRIVATE src ${DITTO_INCLUDE})
  target_link_libraries(test_workflow PRIVATE
    GTest::gtest_main
    nlohmann_json::nlohmann_json
  )
  add_test(NAME TestWorkflow COMMAND test_workflow)

  # Custom test target
  add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_models test_correlation test_ditto_sync test_workflow
    COMMENT "Running all tests"
  )
endif()
