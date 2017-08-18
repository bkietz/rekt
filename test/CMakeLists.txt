cmake_minimum_required(VERSION 3.0)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_executable(tests tests.cpp)
