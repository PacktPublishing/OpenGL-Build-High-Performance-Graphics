=======================================================================================
Compilation and Execution Instructions (Mac OS X and Linux users):
=======================================================================================
This chapter consists of many samples and a single Makefile was written to automate the
compilation process for all demos. We assume that users have already installed the GLFW
library using the command line interface (Ch 1).

In Ubuntu, simply run
   sudo apt-get install glfw

In Mac OS X, simply run
   sudo port install glfw

To compile all the code, run the make command in the terminal in the project directory:
   cd code/Tutorial2/
   make 

Then, a set of executables will be generated in the same directory (type the ls command): 
   Makefile      Tutorial2     gaussian_demo point         triangle
   README.txt    ecg_demo      line          sinusoid_demo

To run each demo, simply type the following commands: 
   ./line
   ./point
   ./triangle 
   ./ecg_demo 

...etc

=======================================================================================
Compilation and Execution Instructions (Windows users):
=======================================================================================
Similar to Ch 1, simply open the individual Visual Studio solution file (.sln) for each 
demo (e.g., Tutorial2/ecg directory for the ECG demo) and click Build (F7).  Run 
the program (F5). 

=======================================================================================
Directory structure:
=======================================================================================
code/Tutorial2/point - contains all source files for the point drawing demo
code/Tutorial2/line - contains all source files for the line drawing demo
code/Tutorial2/triangle - contains all source files for the triangle drawing demo
code/Tutorial2/sinusoid - contains all source files for the sine function drawing demo
code/Tutorial2/gaussian - contains all source files for the Gaussian drawing demo
code/Tutorial2/ecg - contains all source files for the ECG demo

====================================
Sample outputs from the terminal:
====================================
ealabs-MacBook-Pro-2:B02489_02_Codes raymondlo84$ cd code/
ealabs-MacBook-Pro-2:code raymondlo84$ ls
Makefile  Tutorial2
ealabs-MacBook-Pro-2:code raymondlo84$ make
cc -O3 -Wall `pkg-config --cflags glfw3` -o ecg_demo Tutorial2/ecg/main_ecg_demo.cpp -lm `pkg-config --static --libs glfw3`
cc -O3 -Wall `pkg-config --cflags glfw3` -o gaussian_demo Tutorial2/gaussian/main_gaussian_demo.cpp -lm `pkg-config --static --libs glfw3`
cc -O3 -Wall `pkg-config --cflags glfw3` -o sinusoid_demo Tutorial2/sinusoid/main_sinusoid.cpp -lm `pkg-config --static --libs glfw3`
cc -O3 -Wall `pkg-config --cflags glfw3` -o point Tutorial2/point/main_point.cpp  -lm `pkg-config --static --libs glfw3`
cc -O3 -Wall `pkg-config --cflags glfw3` -o line Tutorial2/line/main_line.cpp -lm `pkg-config --static --libs glfw3`
cc -O3 -Wall `pkg-config --cflags glfw3` -o triangle Tutorial2/triangle/main_triangle.cpp -lm `pkg-config --static --libs glfw3`

