#!/bin/bash
ANDROID_SDK_PATH="../../../3rd_party/android/android-sdk-macosx"
OPENCV_SDK_PATH="../../../3rd_party/android/OpenCV-android-sdk"

#initialize the SDK Java library
$ANDROID_SDK_PATH/tools/android update project -p $OPENCV_SDK_PATH/sdk/java -s --target "android-18"
$ANDROID_SDK_PATH/tools/android update project -p . -s --target "android-18" --library $OPENCV_SDK_PATH/sdk/java

sh ./compile_ndk.sh

ant debug
#$ANDROID_SDK_PATH/platform-tools/adb install -r bin/GL3JNIActivity-debug.apk

