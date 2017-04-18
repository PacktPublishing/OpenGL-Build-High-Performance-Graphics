=======================================================================================
Installing the Android SDK and NDK:
=======================================================================================
Here is a summary of the key steps described in Ch 7.  Refer to the book chapter 
for further details.  

1. Run the get_android.sh script inside the 3rd_party directory to download the Android SDK and NDK packages.  
* Note the relative location of the 3rd_party directory. 
* Do not move this folder as the compilation scripts rely on this relative path. 
   cd 9727OS_Final_Code/3rd_party
   chmod +x get_android.sh 
   ./get_android.sh

2. Install the Android SDK 
   ./android/android-sdk-macosx/tools/android

Select Android 4.3.1 (API 18) from the list of packages in addition to the default options and press the Install packages button in the Android SDK Manager screen

Check the installation as follows: 
   ./android/android-sdk-macosx/tools/android list

3. Install the Android NDK 
   ./android/android-ndk-r10e-darwin-x86_64.bin

Note: the directory structure must be preserved for the following scripts to function correctly.

=======================================================================================
Compilation Instructions:
=======================================================================================
We have provided a compilation script to simplify the process (code/simple):

1. Run compile.sh
   cd code/simple
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
jni : source code for all Java Native Interface files (C/C++). The Android.mk and Application.mk files in the directory define all files needed for compilation. 
This is also the directory for the main_simple.cpp file.

src : actual source files for the Android application written in Java. 

res : all resources files, including the application layout, values, and all other graphics related items. See the following link for more detailed descriptions: 
http://developer.android.com/guide/topics/resources/providing-resources.html

All other directories, such as lib, bin, etc will be generated upon compilation. 


-------------------------------
Demo General Structure:
-------------------------------

JNI Native Library calls
  src/com/android/gl3jni/GL3JNILib.java

Implements the OpenGL ES3 GLSurface 
  src/com/android/gl3jni/GL3JNIView.java

Main Class for the Android application which implements the Activity
  src/com/android/gl3jni/GL3JNIActivity.java

Build scripts
  jni/Android.mk
  jni/Application.mk

Main functions and integrated with Native JNI functions
  jni/main_simple.cpp

Variables for the Android package
  res/values/strings.xml

