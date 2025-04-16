cmake_minimum_required(VERSION 3.20)
project(TriangularArbitrageCpp VERSION 1.0 LANGUAGES CXX)

# Set C++ standard and compile flags for production-grade low-latency performance.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -mtune=native -Wall -Wextra -pthread")

# Scan source directory for all .cpp files recursively.
file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

# Create the executable target.
add_executable(TriangularArbitrageCpp ${SOURCES})

# Specify include directories (modern usage via target_include_directories)
target_include_directories(TriangularArbitrageCpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Find and link Threads.
find_package(Threads REQUIRED)
target_link_libraries(TriangularArbitrageCpp PRIVATE Threads::Threads)

# Enable LTO (Link Time Optimization) if supported.
include(CheckIPOSupported)
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_output)
if(ipo_supported)
    set_target_properties(TriangularArbitrageCpp PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO/LTO not supported: ${ipo_output}")
endif()

# Additional compile options for Debug builds: Sanitizers and frame pointers.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(TriangularArbitrageCpp PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(TriangularArbitrageCpp PRIVATE -fsanitize=address,undefined)
endif()

# Set installation target for easy deployment.
install(TARGETS TriangularArbitrageCpp RUNTIME DESTINATION bin)

cmake_minimum_required(VERSION 3.12)
project(TriangularArbitrage)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3 -march=native -Wall -Wextra")

include_directories(include)

add_executable(arbitrage
    src/main.cpp
    src/ExecutionManager.cpp
)
include_directories(${PROJECT_SOURCE_DIR}/include)
