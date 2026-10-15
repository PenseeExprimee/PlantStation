cmake_minimum_required(VERSION 3.14)
project(PlantStation VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library: define the header library
add_library(plant_core INTERFACE)
target_include_directories(plant_core INTERFACE include)
target_compile_options(plant_core INTERFACE -Wall -Wextra -Wpedantic)

# Tests unitaires (download GoogleTest)
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(plant_tests tests/test_sensor.cpp tests/test_ringBuffer.cpp tests/test_MovingAverage.cpp tests/test_lambdas.cpp tests/test_wateringStrategy.cpp tests/test_controller.cpp tests/test_factory.cpp tests/test_handleSubscribers.cpp tests/test_journal.cpp tests/test_alarm.cpp tests/test_eventText.cpp)
add_executable(plant_simulation src/main.cpp)

target_link_libraries(plant_tests PRIVATE plant_core GTest::gtest_main)
target_link_libraries(plant_simulation PRIVATE plant_core)

include(GoogleTest)
gtest_discover_tests(plant_tests)
add_test(NAME simulation_runs COMMAND plant_simulation)