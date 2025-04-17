cmake_minimum_required(VERSION 3.20)
project(TriangularArbitrageSystem VERSION 1.0 LANGUAGES C CXX)

# =====================
# 1. Global Settings
# =====================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# =====================
# 2. Compiler Flags
# =====================
set(COMMON_FLAGS        -Wall -Wextra -Wpedantic)

# Debug
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} ${COMMON_FLAGS} -O0 -g -fsanitize=address,undefined -fno-omit-frame-pointer")
# Release
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${COMMON_FLAGS} -O3 -march=native -mtune=native -flto -ffast-math -fno-exceptions -fno-rtti -DNDEBUG")
# RelWithDebInfo
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${COMMON_FLAGS} -O2 -g -DNDEBUG")
# Test (coverage)
set(CMAKE_CXX_FLAGS_TEST    "${CMAKE_CXX_FLAGS_TEST} ${COMMON_FLAGS} -O1 -g --coverage -fprofile-arcs -ftest-coverage")

# Build configurations exposed in multi-config generators
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;Test" CACHE STRING "Configs" FORCE)

# Link Time Optimization
include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
if(IPO_SUPPORTED)
  message(STATUS "IPO/LTO enabled")
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
else()
  message(WARNING "IPO/LTO not supported: ${IPO_ERROR}")
endif()

# =====================
# 3. Dependencies
# =====================
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(ZeroMQ REQUIRED)

# =====================
# 4. Include Directories
# =====================
include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third_party/eigen
  ${CMAKE_SOURCE_DIR}/third_party
)

# =====================
# 5. Core Module Library
# =====================
set(CORE_SOURCES
    src/core/ConfigLoader.cpp
    src/core/ServiceLocator.cpp
    src/core/Logger.cpp
    src/core/Metrics.cpp
    src/core/execution/ExecutionManager.cpp
    src/core/execution/ExecutionState.cpp
    src/core/messaging/ZeroMQConnectionManager.cpp
    src/core/messaging/ZMQPubSubHandler.cpp
    src/core/risk/RiskManager.cpp
    src/core/router/SmartOrderRouter.cpp
)
add_library(core STATIC ${CORE_SOURCES})
target_include_directories(core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/include/core
)
target_compile_options(core PRIVATE ${COMMON_FLAGS})
target_link_libraries(core PRIVATE ZeroMQ::ZeroMQ)
set_target_properties(core PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include/core"
)

# =====================
# 6. Other Module Libraries
# =====================
file(GLOB_RECURSE SIGNAL_SRC    src/signal/*.cpp)
file(GLOB_RECURSE ROUTER_SRC    src/router/*.cpp)
file(GLOB_RECURSE EXECUTION_SRC src/execution/*.cpp)

add_library(signal    STATIC ${SIGNAL_SRC})
add_library(router    STATIC ${ROUTER_SRC})
add_library(execution STATIC ${EXECUTION_SRC})

foreach(lib IN ITEMS signal router execution)
  target_include_directories(${lib} PUBLIC ${CMAKE_SOURCE_DIR}/include)
  target_compile_options(${lib} PRIVATE ${COMMON_FLAGS})
endforeach()

# =====================
# 7. Main Executable
# =====================
file(GLOB MAIN_SRC src/main.cpp)
add_executable(triangular_arbitrage ${MAIN_SRC})

target_link_libraries(triangular_arbitrage
  PRIVATE
    core
    signal
    router
    execution
    pthread
    m
)

target_compile_definitions(triangular_arbitrage PRIVATE EIGEN_NO_DEBUG)
set_target_properties(triangular_arbitrage PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# =====================
# 8. Tests
# =====================
enable_testing()
file(GLOB_RECURSE TEST_SRC tests/*.cpp)
add_executable(unit_tests ${TEST_SRC})
target_link_libraries(unit_tests PRIVATE
    core signal router execution pthread m
)
add_test(NAME AllUnitTests COMMAND unit_tests)
set_target_properties(unit_tests PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# =====================
# 9. Development Tools
# =====================
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
  file(GLOB_RECURSE ALL_CXX_SRC "src/*.cpp" "include/*.hpp")
  add_custom_target(format
    COMMAND ${CLANG_FORMAT} -i -style=file ${ALL_CXX_SRC}
  )
endif()

# Python helper scripts
add_custom_target(python_scripts
  COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/scripts/run_tests.sh
)

# Docker & Deployment
add_custom_target(docker_build
  COMMAND ${CMAKE_COMMAND} -E echo "Building Docker container..."
  COMMAND docker-compose build
)

add_custom_target(deploy_prod
  COMMAND ${CMAKE_COMMAND} -E echo "Deploying to production cluster..."
  # TODO: integrate CI/CD hooks
)

# =====================
# 10. Installation
# =====================
install(TARGETS triangular_arbitrage DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/config DESTINATION etc/${PROJECT_NAME})

message(STATUS "Root CMake configuration complete.")
