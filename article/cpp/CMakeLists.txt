cmake_minimum_required(VERSION 3.10)
project(herpvs_campaign CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(VISP REQUIRED visp_core visp_io visp_robot)
include_directories(${VISP_INCLUDE_DIRS})

add_executable(test_campaign test_campaign.cpp)
target_link_libraries(test_campaign ${VISP_LIBRARIES})

add_executable(herpvs_campaign herpvs_campaign.cpp)
target_link_libraries(herpvs_campaign ${VISP_LIBRARIES})
