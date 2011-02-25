LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := y
LOCAL_SRC_FILES := hash.c test_hash.c test_queue.c tree.c test_list.c test_stack.c trie.c crc.c test_main.c test_tree.c

LOCAL_FORCE_STATIC_EXECUTABLE := true
LOCAL_CFLAGS +=
LOCAL_C_INCLUDES +=

include $(BUILD_SHARED_LIBRARY)
