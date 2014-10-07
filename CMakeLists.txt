cmake_minimum_required(VERSION 3.0)
enable_testing()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-long-long -std=c++11")

add_subdirectory(HippoMocksTest)

