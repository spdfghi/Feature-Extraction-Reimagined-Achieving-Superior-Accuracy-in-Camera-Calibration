cmake_minimum_required(VERSION 3.10)

project(post_refine VERSION 1.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/Bin)

set(SOURCES
    Src/main.cpp
    Src/sym_refine.h
    Src/application.h
)

# opencv
find_package(OpenCV 4 REQUIRED)
include_directories(${OpenCV4_INCLUDE_DIRS})
link_directories(${OpenCV4_LIBRARY_DIRS})
set(OPENCV_LIBS opencv_core opencv_imgproc opencv_highgui opencv_imgcodecs)

## ceres
#find_package(Ceres REQUIRED)
#include_directories(${CERES_INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    ${OPENCV_LIBS}
#    ${CERES_LIBRARIES}
    )
