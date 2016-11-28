
add_library(HippoMocks INTERFACE)
target_include_directories(HippoMocks INTERFACE .)
if (NOT WIN32)
  target_compile_options(HippoMocks
    INTERFACE
      -Wall -Wextra -pedantic -Wno-long-long -std=c++11
  )
endif()



