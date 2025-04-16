cmake_minimum_required(VERSION 3.20)
project(TriangularArbitrageSystem VERSION 1.0 LANGUAGES C CXX)

# ===================================================
# =============[ 1. Global Settings ]================
# ===================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Optimization and warning flags
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -mtune=native -flto -ffast-math -fno-exceptions -fno-rtti -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -Wall -Wextra -fsanitize=address,undefined -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -DNDEBUG")
set(CMAKE_CXX_FLAGS_TEST    "-O1 -g --coverage -fprofile-arcs -ftest-coverage")

# Build types
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;Test" CACHE STRING "" FORCE)

# ===================================================
# =============[ 2. File & Folder Mapping ]==========
# ===================================================
file(GLOB_RECURSE CORE_SRC src/core/*.cpp)
file(GLOB_RECURSE SIGNAL_SRC src/signal/*.cpp)
file(GLOB_RECURSE TEST_SRC tests/**/*.cpp)
file(GLOB_RECURSE MAIN_SRC src/main.cpp)

# ===================================================
# =============[ 3. Include Directories ]============
# ===================================================
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/interfaces
    ${CMAKE_CURRENT_SOURCE_DIR}/include/core/models
    ${CMAKE_CURRENT_SOURCE_DIR}/include/utils
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/eigen
)
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/third_party/eigen)
# ===================================================
# =============[ 4. Main Executable ]================
# ===================================================
add_executable(xalgo_main
    ${CORE_SRC}
    ${SIGNAL_SRC}
    ${MAIN_SRC}
)

target_compile_definitions(xalgo_main PRIVATE
    BOOST_NO_EXCEPTIONS
    EIGEN_DONT_VECTORIZE=0
    EIGEN_NO_DEBUG
)

target_compile_options(xalgo_main PRIVATE
    -Wall -Wextra -Wpedantic -march=native -mtune=native -fno-exceptions
)

target_link_libraries(xalgo_main
    pthread
    m
    stdc++
)

# Link Time Optimization (IPO)
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)
if(ipo_supported)
    set_target_properties(xalgo_main PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO/LTO not supported: ${ipo_output}")
endif()

# ===================================================
# =============[ 5. Unit/Integration Tests ]=========
# ===================================================
enable_testing()
add_executable(unit_tests
    ${TEST_SRC}
)

target_link_libraries(unit_tests
    pthread
    stdc++
)

add_test(NAME AllUnitTests COMMAND unit_tests)

# ===================================================
# =============[ 6. Development Tools ]==============
# ===================================================
find_program(CLANG_FORMAT "clang-format")
if (CLANG_FORMAT)
    file(GLOB_RECURSE ALL_CXX_SRC "*.cpp" "*.hpp")
    add_custom_target(format
        COMMAND ${CLANG_FORMAT}
        -i
        -style=file
        ${ALL_CXX_SRC}
    )
endif()

# ===================================================
# =============[ 7. Build Output ]===================
# ===================================================
set_target_properties(xalgo_main PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

set_target_properties(unit_tests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/tests
)

# ===================================================
# =============[ 8. Python Integration ]=============
# ===================================================
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_custom_target(python_scripts
    COMMAND Python3::Interpreter ${CMAKE_SOURCE_DIR}/scripts/run_tests.sh
)

# ===================================================
# =============[ 9. Docker & Deployment ]============
# ===================================================
add_custom_target(docker_build
    COMMAND ${CMAKE_COMMAND} -E echo "Building Docker container..."
    COMMAND docker-compose build
)

add_custom_target(deploy_prod
    COMMAND ${CMAKE_COMMAND} -E echo "Deploying to production cluster..."
    # Add CI/CD hooks here
)

# ===================================================
# =============[ 10. Summary Output ]================
# ===================================================
message(STATUS "Build Configuration Complete")
message(STATUS "Core Sources: ${CORE_SRC}")
message(STATUS "Signal Sources: ${SIGNAL_SRC}")
message(STATUS "Main: ${MAIN_SRC}")
message(STATUS "Tests: ${TEST_SRC}")
message(STATUS "Using Eigen from: ${CMAKE_CURRENT_SOURCE_DIR}/third_party/eigen")
