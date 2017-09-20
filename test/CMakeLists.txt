cmake_minimum_required(VERSION 3.0)
project(rekt.tests CXX)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(CMAKE_CXX_STANDARD 14)

add_library(Catch STATIC catch.cpp)

add_executable(tests tests.cpp)
target_link_libraries(tests Catch)
