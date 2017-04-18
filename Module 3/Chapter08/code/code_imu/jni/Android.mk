LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := libgl3jni
LOCAL_CFLAGS    := -Werror -O3
LOCAL_SRC_FILES := main_sensor.cpp Sensor.cpp
LOCAL_LDLIBS    := -llog -lGLESv3 -lm 

include $(BUILD_SHARED_LIBRARY)

