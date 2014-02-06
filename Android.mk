LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := y
LOCAL_SRC_FILES := \
	crc.c \
	list.c \
	tree.c \
	trie.c \
	hash.c \
	mempool.c \
	lru.c \
	msgq.c \
	libmain.c

#LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_CFLAGS += -Wall -Werror -DCONFIG_IGNORE_CONFIG
LOCAL_C_INCLUDES +=

include $(BUILD_SHARED_LIBRARY)
