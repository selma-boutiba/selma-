# Your CMakeLists.txt, for the article version of tutorial-viewer4.cpp.
# Two additions: the folder of campaign.h (article/cpp), and, for Visual
# Studio, three definitions that keep <windows.h> and <cmath> compatible.
cmake_minimum_required(VERSION 3.10)

project(tutorial-viewer4-article)

find_package(VISP REQUIRED)

include_directories(${VISP_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../cpp)   # campaign.h

if(MSVC)
  add_definitions(-DNOMINMAX -D_USE_MATH_DEFINES -D_CRT_SECURE_NO_WARNINGS)
endif()

add_executable(tutorial-viewer4-article tutorial-viewer4-article.cpp)
target_link_libraries(tutorial-viewer4-article ${VISP_LIBRARIES})
