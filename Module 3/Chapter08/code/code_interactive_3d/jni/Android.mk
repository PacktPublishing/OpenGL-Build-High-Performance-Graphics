LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libgl3jni
LOCAL_CFLAGS    := -Werror
LOCAL_SRC_FILES := main.cpp
LOCAL_LDLIBS    := -llog -lGLESv3
LOCAL_C_INCLUDES := /opt/local/include /usr/local/include

include $(BUILD_SHARED_LIBRARY)

