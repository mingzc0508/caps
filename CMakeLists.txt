CMAKE_MINIMUM_REQUIRED(VERSION 3.2)
project(caps)
set(VERSION 2.0.0)

include (${CUSTOM_CMAKE_MODULES}/common.mk)

option(BUILD_DEBUG "debug or release" OFF)
option(BUILD_TEST "build test programs" OFF)

set(CMAKE_CXX_STANDARD 11)
if (BUILD_DEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
endif()

add_library(caps SHARED src/caps.cpp src/member.h include/caps.h)
target_include_directories(caps PRIVATE
  include
)

# install include files.
file(GLOB caps_HEADERS
  include/caps.h
)
install(FILES ${caps_HEADERS}
  DESTINATION include/caps
  PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
)
install(TARGETS caps
  LIBRARY DESTINATION lib
)

if (BUILD_TEST)
findPackage(gtest REQUIRED
  HINTS ${gtestPrefix}
  HEADERS gtest/gtest.h
  STATIC_LIBS gtest
)
file(GLOB caps_test_SOURCES
  tests/*.cpp
)
add_executable(caps-tests ${caps_test_SOURCES})
target_include_directories(caps-tests PRIVATE
  include
  src
  ${gtest_INCLUDE_DIRS}
)
target_link_libraries(caps-tests
  caps
  ${gtest_LIBRARIES}
)
endif(BUILD_TEST)
