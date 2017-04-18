=======================================================================================
Setting up the Development Environment:
=======================================================================================
Please follow the detailed installation instructions in Chapter 1 before proceeding. 

=======================================================================================
Compilation and Execution Instructions (Mac OS X and Linux users):
=======================================================================================
1. To compile the code, go inside the “code” directory (using the cd command) 
and run the following commands in the terminal:
   chmod +x compile.sh
   ./compile.sh

Alternatively, you can copy and paste the following command to the terminal: 
   gcc -Wall `pkg-config --cflags glfw3` -o main Tutorial1/main.cpp `pkg-config --static --libs glfw3`

This will generate an executable called main in the current directory.
 
2. To run the code, simply run the following command:
   ./main

=======================================================================================
Compilation and Execution Instructions (Windows users):
=======================================================================================
Simply open the Tutorial1.sln Visual Studio solution file and click Build (F7).  Run 
the program (F5). 

=======================================================================================
Directory structure:
=======================================================================================
code/Tutorial1/main.cpp - contains the source code for the first OpenGL demo
code/Tutorial1.sln - solution file for Microsoft Visual Studio 
code/Tutorial1/Tutorial1.vcxproj - project file for Microsoft Visual Studio
code/Tutorial1/Tutorial1.vcxproj.filters - basic filters/settings to organize the code
files for more complex projects

The Microsoft Visual Studio files contain the project settings for compiling and executing 
the OpenGL demo in Windows. These files will be generated automatically with CMake 
in later chapters as the complexity of the code increases.

