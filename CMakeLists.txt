cmake_minimum_required(VERSION 2.6)

project(graph_cuts)

FIND_PACKAGE(OpenCV REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

file(
GLOB_RECURSE
source_files
*.[cht]pp
)

ADD_DEFINITIONS( -Wall -Wextra -std=c++11 -Wno-comment -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -fopenmp -DCORRECTION -DNDEBUG)

add_executable(
  graph_cuts
  ${source_files}
)

TARGET_LINK_LIBRARIES(graph_cuts -lm -ldl -lGLEW ${OpenCV_LIBRARIES} -fopenmp)

