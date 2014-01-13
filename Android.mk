LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := y
LOCAL_SRC_FILES := crc.c libmain.c trie.c hash.c mempool.c tree.c

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_CFLAGS += -Wall -Werror
LOCAL_C_INCLUDES +=

include $(BUILD_SHARED_LIBRARY)
