=======================================================================================
IMPORTANT: MUST INSTALL OPENCV for Android for this chapter
=======================================================================================
1. To install OpenCV on Android, run the get_opencv.sh script in the 3rd_party folder in 
the top-level project directory.
   cd 9727OS_Final_Code/3rd_party 
   chmod +x get_opencv.sh
   ./get_opencv.sh

2. Install the OpenCV Manager on the device (make sure you are in the 3rd_party path).
   ./android/android-sdk-macosx/tools/android/adb install android/OpenCV-android-sdk/apk/OpenCV_3.0.0_Manager_3.00_armeabi-v7a.apk

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
We have provided a compilation script to simplify the process 
(inside the code/opencv_demo_1 and code/opencv_demo_2 folders):

1. Run compile.sh
   cd code/opencv_demo_1
   chmod +x compile.sh
   ./compile.sh

2. Make sure the phone is connected to the computer through the USB port and the 
Developer option (USB Debugging mode) is enabled (as described in Ch 7). 

3. Run the install.sh script to install the program on the device
   chmod +x install.sh
   ./install.sh

=======================================================================================
Directory Structure:
=======================================================================================
[folder]: [description]
/jni : source code for all Java Native Interface files (C/C++). The Android.mk and Application.mk files in the directory define all files needed for compilation. 
This is also the directory for the main.cpp file.

/src : actual source files for the Android application written in Java. 

/res : all resources files, including the application layout, values, and all other graphics related items. See the following link for more detailed descriptions: 
http://developer.android.com/guide/topics/resources/providing-resources.html

All other directories, such as lib, bin, etc will be generated upon compilation. 


-------------------------------
OpenCV Demo General Structure:
-------------------------------
JNI Native Library calls
  src/com/android/gl3jni/GL3JNILib.java

Implements the OpenGL ES3 GLSurface and touch interfaces
  src/com/android/gl3jni/GL3JNIView.java

Main Class for the Android application which implements the Activity
And other sensor inputs
  src/com/android/gl3jni/GL3OpenCVDemo.java

Layout for the enabling the JavaCameraView
  res/layout/ar.xml

Build scripts
  jni/Android.mk
  jni/Application.mk

Implmentations for the shader programs
  jni/Shader.cpp
  jni/Shader.hpp

Implementations of the texture mapping
  jni/Texture.cpp
  jni/Texture.hpp

Video rendering using Texture mapping functions
  jni/VideoRenderer.cpp
  jni/VideoRenderer.hpp

Main functions and integrated with Native JNI and OpenCV functions
  jni/main.cpp

Implementations for overlaying the OpenGL 3D graphics onto the video feed
  jni/AROverlayRenderer.cpp
  jni/AROverlayRenderer.hpp


