=======================================================================================
Setting up the Development Environment:
=======================================================================================
Download the pre-requisite libraries from the following websites:
1. GLEW (glew-1.10.0): 
   http://sourceforge.net/projects/glew/files/glew/1.10.0/glew-1.10.0-win32.zip
2. GLM (glm-0.9.5.4): 
   http://sourceforge.net/projects/ogl-math/files/glm-0.9.5.4/glm-0.9.5.4.zip
3. SOIL: 
   http://www.lonesock.net/files/soil.zip
4. OpenCV (opencv-2.4.9): 
   http://sourceforge.net/projects/opencvlibrary/files/opencv-win/2.4.9/opencv-2.4.9.exe

For Linux or Mac users
1. GLEW (glew-1.10.0): 
   https://sourceforge.net/projects/glew/files/glew/1.10.0/glew-1.10.0.tgz
2. GLM (glm-0.9.5.4): 
   http://sourceforge.net/projects/ogl-math/files/glm-0.9.5.4/glm-0.9.5.4.zip
3. SOIL: 
   http://www.lonesock.net/files/soil.zip
4. OpenCV (opencv-2.4.9): 
   http://sourceforge.net/projects/opencvlibrary/files/opencv-unix/2.4.9/opencv-2.4.9.zip

Please follow the detailed installation instructions in Chapter 4 before proceeding. 

=======================================================================================
Compilation and Execution Instructions (Mac OS X and Linux users):
=======================================================================================
To compile the code, run the make command in each of the project directory.

For example, type the following commands to run the simple demo: 
   cd code/code_simple
   make

To run the executable, simply type
   cd code/code_simple
   ./main

=======================================================================================
Compilation and Execution Instructions (Windows users):
=======================================================================================
Follow the detailed instructions in Ch 4 to generate the Visual Studio files
using CMake. Then, compile and run the demo as in previous chapters.  

=======================================================================================
Directory structure:
=======================================================================================
code_simple - directory for the simple demo 
code_simple/Makefile - makefile for code compilation
code_simple/CMakeList.txt - configuration file for CMake (Windows only)
code_simple/simple.frag - fragment shader source file
code_simple/simple.vert - vertex shader source file
code_simple/main.cpp - the main source code for the demo

code_image - directory for the texture mapping demo to render images
code_image/Makefile - makefile for the code compilation
code_image/CMakeList.txt - configuration file for CMake (Windows only)
code_image/texture.frag - fragment shader source file for texture mapping
code_image/texture_sobel.frag - fragment shader source file with Sobel filter implementation
code_image/transform.vert - vertex shader source file 
code_image/main.cpp - the main source code for the demo

code_video - directory for the texture mapping and video playback demo
code_video/Makefile - makefile for the code compilation
code_video/CMakeList.txt - configuration file for CMake (Windows only)
code_video/texture.frag - fragment shader source file for texture mapping
code_video/texture_sobel.frag - fragment shader source file with Sobel filter implementation
code_video/transform.vert - vertex shader source file 
code_video/main.cpp - the main source code for the demo
code_video/movies - directory for all sample movies
code_video/movies/video.mov - sample with authors panning the camera
code_video/movies/OCT_finger.avi - sample movie for OCT volumetric dataset of a human finger
code_video/movies/OCT-scar.mov - sample movie for 3-D rendering of a human scar (PS-OCT)

common - contains all shared files
common/common.h - the common headers
common/texture.cpp - the texture mapping related implementations
common/texture.hpp - the texture mapping related headers
common/controls.cpp - the virtual camera related implementations
common/controls.hpp - the virtual camera related headers
common/shader.cpp - the shader related implementations
common/shader.hpp - the shader related headers


