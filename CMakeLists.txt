cmake_minimum_required(VERSION 3.0)
enable_testing()

project(HippoMocks)

add_subdirectory(HippoMocks)
add_subdirectory(HippoMocksTest)

install(FILES ${PROJECT_SOURCE_DIR}/HippoMocks/hippomocks.h
        DESTINATION include/)
