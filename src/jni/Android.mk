# A simple test for the minimal standard C++ library
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := aproxy
LOCAL_CFLAGS := -I daemon/include
LOCAL_SRC_FILES := daemon/main.c daemon/nat.c  daemon/proxy_container.c daemon/proxy.c daemon/proxy_noblock.c daemon/session.c \
                   daemon/http.c daemon/http_parser.c daemon/rwbuff.c daemon/cache.c  daemon/log.c daemon/input_stream.c \
                   daemon/conf.c daemon/common.c 
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := htest
LOCAL_CFLAGS := -DTEST_PARSER
LOCAL_SRC_FILES := daemon/http_parser.c 
include $(BUILD_EXECUTABLE)
