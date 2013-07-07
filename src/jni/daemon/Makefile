TOPDIR=.
NOPROGCOPY=1

include $(TOPDIR)/prerules.Make

USEGCC=gcc
LDGPLUSPLUS=g++

ifeq (${PLATFORM}, Darwin)
OBJS= main.o nat.o scanner.o fccheck.o proxy_container.o proxy.o proxy_noblock.o session.o spool.o\
      http.o ftp.o pop3.o smtp.o http_sysinfo_sender.o\
      rwbuff.o cache.o http_parse.o  handle_command.o sysinfo_mac.o 
TARGET_PROG_EXTRAS=../util/libfctcommon.a  $(TOPDIR)/src/thirdparty/pcre/.libs/libpcre.a 
else
OBJS= main.o nat.o  proxy_mgr.o proxy_util.o proxy_core.o session.o \
      http.o http_parser.o\
      rwbuff.o cache.o  log.o input_stream.o  conf.o  common.o 
#TARGET_PROG_EXTRAS=../util/libfctcommon.a  $(TOPDIR)/src/thirdparty/pcre/.libs/libpcre.a 
endif

EXTRA_INCS += -I include -I . -D$(ARCH)  

TARGET_PROG=fctproxyd
TARGET_PROG_OBJECTS=$(OBJS) 
#TARGET_PROG_EXTRA_LDFLAGS=-lz -lcrypto -lstdc++  
 
#TARGET_PROG1=proxytest
#TARGET_PROG1_OBJECTS=proxytest.o

#TARGET_PROG=sysinfotest
#TARGET_PROG_OBJECTS=sysinfo_mac.o 
#TARGET_PROG_EXTRAS=../util/libfctcommon.a  $(TOPDIR)/src/thirdparty/pcre/.libs/libpcre.a 
#TARGET_PROG_EXTRA_LDFLAGS=-lz -lcrypto -lstdc++ -lsqlite3

include $(TOPDIR)/rules.Make

