cmake_minimum_required(VERSION 3.10)
project(herpvs_campaign CXX)
# C++14 is enough for this code. ViSP >= 3.6 built with GCC needs C++17 for
# its own headers; Visual Studio 2015 (MSVC 19.0) supports C++14 at most.
if(NOT CMAKE_CXX_STANDARD)
  if(MSVC AND MSVC_VERSION LESS 1910)
    set(CMAKE_CXX_STANDARD 14)
  else()
    set(CMAKE_CXX_STANDARD 17)
  endif()
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)   # single-configuration generators: optimise by default
endif()
if(MSVC)
  # keep <windows.h> from defining min/max macros; define M_PI in <cmath>
  add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS)
endif()

find_package(VISP REQUIRED visp_core visp_io visp_robot)
include_directories(${VISP_INCLUDE_DIRS})

add_executable(test_campaign test_campaign.cpp)
target_link_libraries(test_campaign ${VISP_LIBRARIES})

add_executable(herpvs_campaign herpvs_campaign.cpp)
target_link_libraries(herpvs_campaign ${VISP_LIBRARIES})
