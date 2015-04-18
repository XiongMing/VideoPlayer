LOCAL_PATH := $(call my-dir)

#下面编译android2.3 libfsplayer23_a
include $(CLEAR_VARS)

LOCAL_CFLAGS := -D__STDC_CONSTANT_MACROS
LOCAL_CFLAGS += -g

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/../libffmpeg \
    $(LOCAL_PATH)/../libffmpeg_so \
    $(LOCAL_PATH)/../jni	\
    $(LOCAL_PATH)/android23	\
    $(LOCAL_PATH)/../include \
    $(LOCAL_PATH)/../libbreakpad/include/breakpad \
    $(LOCAL_PATH)/../libbreakpad/breakpad_android/jni/src/common/android/include
				
    
LOCAL_STATIC_LIBRARIES := cpufeatures

LOCAL_SRC_FILES += \
    uthread.cpp			\
    uqueue.cpp			\
    udecoder.cpp		\
    udecoder_audio.cpp	\
    uallocator.cpp		\
    urenderer_opensl_es.cpp	\
    uparser.cpp			\
    uplayer.cpp			\
    udecoder_video.cpp	\
    ulog.cpp			\
    urenderer_video.cpp	\
    ugraphics.cpp		\
    umsg_queue.cpp		\
    utimer.cpp			\
    ulock.cpp			\
    usemaphore.cpp \
    ucache_protocol.c \
    ucache_file.cpp \
    ucrash_handler.cpp
    
#LOCAL_STATIC_LIBRARIES := libavcodec libavformat libavutil libpostproc libswscale libavfilter


LOCAL_MODULE := libfsplayer23_a

include $(BUILD_STATIC_LIBRARY)

$(call import-module, android/cpufeatures)