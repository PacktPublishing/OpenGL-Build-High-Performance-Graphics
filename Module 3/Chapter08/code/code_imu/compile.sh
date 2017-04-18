#!/bin/bash
ANDROID_SDK_PATH="../../../3rd_party/android/android-sdk-macosx"
ANDROID_NDK_PATH="../../../3rd_party/android/android-ndk-r10e"

$ANDROID_SDK_PATH/tools/android update project -p . -s --target "android-18"
$ANDROID_NDK_PATH/ndk-build
ant debug

#$ANDROID_SDK_PATH/platform-tools/adb install -r bin/GL3JNIActivity-debug.apk

