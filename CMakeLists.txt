cmake_minimum_required(VERSION 3.16)
project(QuantBacktester VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")

# Include directories
include_directories(src/cpp)

# Add main tick engine executable
add_executable(QuantTickEngine 
    src/cpp/main.cpp
    src/cpp/tick_engine.cpp
    src/cpp/order_book.cpp
    src/cpp/market_impact.cpp
)

# Add test target
enable_testing()
add_executable(cpp_tests
    tests/cpp/test_main.cpp
    tests/cpp/test_tick_engine.cpp
)
target_link_libraries(cpp_tests pthread)