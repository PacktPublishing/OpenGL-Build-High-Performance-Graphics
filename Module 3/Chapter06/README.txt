=======================================================================================
Compilation and Execution Instructions:
=======================================================================================
To compile the code in Windows, please use CMake and follow similar 
instructions as provided in Ch 4.

To compile the code in Linux/Mac OS X, run the make command in the project 
directory (3d_model_demo) which contains the Makefile: 
   cd code/3d_model_demo
   make

To run the code, type the following command: 
   ./main

Alternatively, you can load different model using
   ./main obj_samples/sibenik.obj

==============================================================================
Directory structure:
==============================================================================
3d_model_demo/Makefile - makefile for the code compilation
3d_model_demo/CMakeList.txt - configuration file for CMake (Windows only)
3d_model_demo/main.cpp - source code for the demo
3d_model_demo/pointcloud.frag - fragment shader source file
3d_model_demo/pointcloud.vert - vertex shader source file

3d_model_demo/obj_samples - contains all sample .obj files (3D models)

common - contains all shared files
common/common.h - the common headers
common/texture.cpp - the texture mapping related implementations
common/texture.hpp - the texture mapping related headers
common/controls.cpp - the virtual camera related implementations
common/controls.hpp - the virtual camera related headers
common/ObjLoader.cpp - the 3d model loading related implementations
common/ObjLoader.hpp - the 3d model loading related headers
common/shader.cpp - the shader related implementations
common/shader.hpp - the shader related headers





