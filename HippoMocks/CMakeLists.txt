
add_library(HippoMocks INTERFACE)
target_include_directories(HippoMocks INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)
target_compile_features(HippoMocks INTERFACE cxx_std_11)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/hippomocks.h
        DESTINATION include/)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/detail
        DESTINATION include)

install(
  TARGETS
    HippoMocks
  EXPORT
    HippoMocksTargets
  DESTINATION
    ./
)
