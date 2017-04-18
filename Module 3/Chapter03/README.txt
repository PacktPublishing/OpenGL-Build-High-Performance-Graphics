=======================================================================================
Compilation and Execution Instructions (Mac OS X and Linux users):
=======================================================================================
To compile the code, run the make command within each demo project directory:
   cd code/gaussian
   make 

   cd code/mcml
   make

Then, a set of executables will be generated inside each directory: 
   code/gaussian/main
   code/mcml/main

To run each demo, simply type the following commands: 
   cd code/gaussian
   ./main

or

   cd code/mcml
   ./main

=======================================================================================
Compilation and Execution Instructions (Windows users):
=======================================================================================
Similar to Ch 1, simply open the individual Visual Studio solution file (.sln) for each 
demo (e.g., code/gaussian directory for Gaussian demo) and click Build (F7).  Run 
the program (F5). 

=======================================================================================
Directory structure:
=======================================================================================
code/mcml/main.cpp - contains all source files for the MCML demo
code/mcml/MCML_output.txt - the simulation output file for MCML
code/mcml/Makefile - makefile for compiling the code

code/gaussian/main.cpp - contains all source files for the Gaussian demo
code/gaussian/Makefile - makefile for compiling the code

Also, we have provided the .sln, .vcxproj files for convenience. Double-click
on any of the *.sln files to open Visual Studio 2013 and follow the on-screen 
instructions in Chapter 1 or simply press F7.





