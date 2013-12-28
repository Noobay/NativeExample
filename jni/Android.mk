LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := NativeActivity
LOCAL_SRC_FILES := ../../NativeActivity/obj/local/armeabi/libNativeActivity.a

include $(PREBUILT_STATIC_LIBRARY)


include $(CLEAR_VARS)

LOCAL_MODULE    := NativeExample
LOCAL_SRC_FILES := NativeExample.cpp
LOCAL_CPPFLAGS  := -DNULL=0 -frtti
LOCAL_C_INCLUDES += ../NativeActivity/jni
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2

LOCAL_WHOLE_STATIC_LIBRARIES  := libNativeActivity

include $(BUILD_SHARED_LIBRARY)