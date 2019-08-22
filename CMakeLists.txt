cmake_minimum_required(VERSION 3.0)

enable_testing()

project(HippoMocks)

include(CTest)

add_subdirectory(HippoMocks)
if (BUILD_TESTING)
  add_subdirectory(HippoMocksTest)
endif()

include(CMakePackageConfigHelpers)

set(HIPPOMOCKS_CMAKE_CONFIG_DESTINATION "cmake/")

configure_package_config_file(
    ${CMAKE_CURRENT_LIST_DIR}/CMake/HippoMocksConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/HippoMocksConfig.cmake
  INSTALL_DESTINATION
    ${HIPPOMOCKS_CMAKE_CONFIG_DESTINATION}
)

install(
  EXPORT
    HippoMocksTargets
  NAMESPACE
    HippoMocks::
  DESTINATION
    ${HIPPOMOCKS_CMAKE_CONFIG_DESTINATION}
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/HippoMocksConfig.cmake"
  DESTINATION
    ${HIPPOMOCKS_CMAKE_CONFIG_DESTINATION}
)
