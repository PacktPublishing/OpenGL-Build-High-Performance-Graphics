cmake_minimum_required (VERSION 2.8)

set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(PROGRAM_PATH "C:/Program Files \(x86\)")
set(OpenCV_DIR ${PROGRAM_PATH}/opencv/build)

project (code_image)

#modify these path based on your configuration

#OpenCV
find_package(OpenCV REQUIRED )
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

INCLUDE_DIRECTORIES(${PROGRAM_PATH}/glm)

INCLUDE_DIRECTORIES(${PROGRAM_PATH}/glew-1.10.0/include)
LINK_DIRECTORIES(${PROGRAM_PATH}/glew-1.10.0/lib/Release/Win32)

INCLUDE_DIRECTORIES(${PROGRAM_PATH}/glfw-3.0.4/include)
LINK_DIRECTORIES(${PROGRAM_PATH}/glfw-3.0.4/lib)

INCLUDE_DIRECTORIES(${PROGRAM_PATH}/Simple\ OpenGL\ Image\ Library/src)
LINK_DIRECTORIES(${PROGRAM_PATH}/Simple\ OpenGL\ Image\ Library/lib)

add_subdirectory(../common common)

add_executable (main main.cpp)

target_link_libraries (main LINK_PUBLIC shader controls texture glew32s glfw3 opengl32 SOIL)
