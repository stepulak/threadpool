cmake_minimum_required(VERSION 3.0)
project(threadpool_test)

find_package(Boost COMPONENTS unit_test_framework REQUIRED)
set(CMAKE_CXX_STANDARD 17)

add_executable(threadpool_test threadpool_test.cpp)
target_link_libraries(threadpool_test Boost::unit_test_framework)

enable_testing()
add_test(threadpool_test threadpool_test)