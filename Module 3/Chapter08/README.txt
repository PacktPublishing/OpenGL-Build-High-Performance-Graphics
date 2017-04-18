=======================================================================================
Installing the Android SDK and NDK:
=======================================================================================
Refer to the Ch 7 README and book chapter for details. 
Note: We assume the relative location of the 3rd_party folder is the same in Ch 7-9.

=======================================================================================
Installing the GLM header library (Same as Ch 4):
=======================================================================================
In Mac OS X, we can install GLM with MacPorts using the following command: 
   sudo port install glm

In Linux, we can install GLM with apt-get using the following command: 
   sudo apt-get install libglm-dev

=======================================================================================
Compilation Instructions:
=======================================================================================
We have provided a compilation script to simplify the process (inside the code/code_imu and code/code_interactive_3d folders):

1. Run compile.sh (e.g., inside the code/code_interactive_3d folder)
   cd code/code_interactive_3d
   chmod +x compile.sh
   ./compile.sh

2. Make sure the phone is connected to the computer through the USB port and the 
Developer option (USB Debugging mode) is enabled (as described in Ch 7). 

3. Run the install.sh script to install the program on the device
   chmod +x install.sh
   ./install.sh

=======================================================================================
Directory structure:
=======================================================================================
[folder]: [description]
jni : source code for all Java Native Interface files (C/C++). The Android.mk and Application.mk files in the directory define all files needed for compilation. 
This is also the directory for the main.cpp or main_sensor.cpp file.

src : actual source files for the Android application written in Java. 

res : all resources files, including the application layout, values, and all other graphics related items. See the following link for more detailed descriptions: 
http://developer.android.com/guide/topics/resources/providing-resources.html

All other directories, such as lib, bin, etc will be generated upon compilation. 

-------------------------------
IMU Demo General Structure:
-------------------------------
JNI Native Libs
  src/com/android/gl3jni/GL3JNILib.java

Implements the OpenGL ES3 GLSurface and touch interfaces
  src/com/android/gl3jni/GL3JNIView.java

Main Class for the Android application which implements the Activity
And other sensor inputs
  src/com/android/gl3jni/GL3JNIActivity.java

Build scripts
  jni/Android.mk
  jni/Application.mk


Class for handling the Sensor data buffers
  jni/Sensor.cpp
  jni/Sensor.h

Main functions and integrated with Native JNI functions
  jni/main_sensor.cpp

-------------------------------
Interactive Demo General Structure:
-------------------------------

JNI Native Library calls
  src/com/android/gl3jni/GL3JNILib.java

Implements the OpenGL ES3 GLSurface and touch interfaces
  src/com/android/gl3jni/GL3JNIView.java

Main Class for the Android application which implements the Activity
And other sensor inputs
  src/com/android/gl3jni/GL3JNIActivity.java

Build scripts
  jni/Android.mk
  jni/Application.mk

Main functions and integrated with Native JNI functions
  jni/main.cpp


