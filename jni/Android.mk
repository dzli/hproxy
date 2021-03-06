# A simple test for the minimal standard C++ library
#

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := hproxy
LOCAL_CFLAGS := -I daemon/include
LOCAL_SRC_FILES := daemon/main.c daemon/nat.c  daemon/proxy_mgr.c daemon/proxy_util.c daemon/proxy_core.c daemon/session.c \
                   daemon/http.c daemon/http_parser.c daemon/rwbuff.c daemon/cache.c  daemon/log.c daemon/input_stream.c \
                   daemon/conf.c daemon/common.c 
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := htest
LOCAL_CFLAGS := -DTEST_PARSER
LOCAL_SRC_FILES := daemon/http_parser.c 
include $(BUILD_EXECUTABLE)
