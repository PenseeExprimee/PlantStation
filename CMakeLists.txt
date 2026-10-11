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
add_executable(plant_tests tests/test_sensor.cpp)
add_executable(tampon_tests tests/test_ringBuffer.cpp)
add_executable(filter_tests tests/test_MovingAverage.cpp)
add_executable(lambdas_tests tests/test_lambdas.cpp)
add_executable(wateringStrategy_tests tests/test_wateringStrategy.cpp)
add_executable(wateringController_tests tests/test_controller.cpp)
add_executable(factory_tests tests/test_factory.cpp)
add_executable(subscribers_tests tests/test_handleSubscribers.cpp)
add_executable(eventJournal_tests tests/test_journal.cpp)

target_link_libraries(plant_tests PRIVATE plant_core GTest::gtest_main)
target_link_libraries(tampon_tests PRIVATE plant_core GTest::gtest_main)
target_link_libraries(filter_tests PRIVATE plant_core GTest::gtest_main)
target_link_libraries(lambdas_tests PRIVATE plant_core GTest::gtest_main)
target_link_libraries(wateringStrategy_tests PRIVATE plant_core GTest::gtest_main)
target_link_libraries(wateringController_tests PRIVATE plant_core GTest::gtest_main)
target_link_libraries(factory_tests PRIVATE plant_core GTest::gtest_main)
target_link_libraries(subscribers_tests PRIVATE plant_core GTest::gtest_main)
target_link_libraries(eventJournal_tests PRIVATE plant_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(plant_tests)
gtest_discover_tests(tampon_tests)
gtest_discover_tests(filter_tests)
gtest_discover_tests(lambdas_tests)
gtest_discover_tests(wateringStrategy_tests)
gtest_discover_tests(factory_tests)
gtest_discover_tests(subscribers_tests)
gtest_discover_tests(wateringController_tests)
gtest_discover_tests(eventJournal_tests)
