# define new project
PROJECT(AR)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.0 FATAL_ERROR)

if(UNIX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif(UNIX)

# Set shared lib build for the rest
SET(BUILD_SHARED_LIBS ON)

# Find dependencies
SET(EIGEN_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/ext/Eigen-3.1.2")

# Set header and source files
SET(AR_SOURCES
    src/main.cpp
        src/MarkerTracker.cpp src/MarkerTracker.h
        src/PoseEstimation.cpp src/PoseEstimation.h
        src/DrawPrimitives.cpp src/DrawPrimitives.h
        src/RenderText.cpp src/RenderText.h
        src/shader_utils.cpp src/shader_utils.h
        src/Mesh.cpp src/Mesh.h
        src/ObjectParser.cpp src/ObjectParser.h)

# define executable
ADD_EXECUTABLE(${PROJECT_NAME} ${AR_SOURCES})

find_package(OpenCV REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

find_package(OPENGL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

find_package(GLUT REQUIRED)
target_link_libraries(${PROJECT_NAME} ${GLUT_LIBRARIES})

find_package(GLM REQUIRED)
target_link_libraries(${PROJECT_NAME} ${GLM_LIBRARIES})

find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} ${GLEW_LIBRARIES})

include(FindFreeType)
find_package(FREETYPE REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${FREETYPE_LIBRARIES})

# define additional include directories and linking targets
INCLUDE_DIRECTORIES(${EIGEN_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${OPENGL_glu_LIBRARY} -lassimp)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)
include_directories(${GLFW_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLFW_LIBRARIES})

