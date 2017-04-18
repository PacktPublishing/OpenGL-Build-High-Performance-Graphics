=======================================================================================
Compilation and Execution Instructions:
=======================================================================================
To compile the code in Windows, please use CMake and follow similar 
instructions as provided in Ch 4.

To compile the code in Linux/Mac OS X, run the make command in the project (code) 
directory which contains the Makefile file

The 3rd_party folder contains the OpenNI2 drivers and SDK to capture raw data
from range-sensing cameras. See Ch 5 for detailed setup instructions.  
Users without such a device can still experiment with the source code 
using the binary raw data files (color_frame0.bin, depth_frame0.bin) provided. 

==============================================================================
Directory structure:
==============================================================================
code/Makefile - makefile for the code compilation
code/CMakeList.txt - configuration file for CMake (Windows only)
code/main.cpp - source code for the sample demo
code/pointcloud.frag - fragment shader source file
code/pointcloud.vert - vertex shader source file
code/color_frame0.bin - binary file for the color image
code/depth_frame0.bin - binary file for the depth image

common - contains all shared files
common/common.h - the common headers
common/texture.cpp - the texture mapping related implementations
common/texture.hpp - the texture mapping related headers
common/controls.cpp - the virtual camera related implementations
common/controls.hpp - the virtual camera related headers
common/shader.cpp - the shader related implementations
common/shader.hpp - the shader related headers
