LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libgl3jni
LOCAL_CFLAGS    := -Werror
#for simplified demo
LOCAL_SRC_FILES := main_simple.cpp
LOCAL_LDLIBS    := -llog -lGLESv3

include $(BUILD_SHARED_LIBRARY)

