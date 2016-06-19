# define new project
PROJECT(MAR)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

if(UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(UNIX)

# Set shared lib build for the rest
SET(BUILD_SHARED_LIBS ON)

# Find dependencies
SET(EIGEN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/ext/Eigen-3.1.2")

# Set header and source files
SET(MAR_SOURCES
    src/main.cpp
        src/MarkerTracker.cpp src/MarkerTracker.h
        src/PoseEstimation.cpp src/PoseEstimation.h
        src/DrawPrimitives.cpp src/DrawPrimitives.h)

# define executable
ADD_EXECUTABLE(${PROJECT_NAME} ${MAR_SOURCES})

find_package(OpenCV REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

find_package(OPENGL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

# define additional include directories and linking targets
INCLUDE_DIRECTORIES( ${EIGEN_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OPENGL_glu_LIBRARY} ${GLFW_LIBRARIES})


find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES})