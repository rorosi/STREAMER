####################################################
BUILD_HOME := /home/server/
SYSTEM := SERVER
APP_DIR := $(BUILD_HOME)/virtual

MODEL_NAME := 
LIB_DIR	   = $(APP_DIR)/lib
BIN_DIR    = $(APP_ROOT_PATH)/../bin

#####################################################
APP_ROOT_PATH	= $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
APP_LIB_PATH	= $(APP_ROOT_PATH)/../lib
APP_INC_PATH	= $(APP_ROOT_PATH)/../include

APP_BUILD_YEAR = $(shell date +%Y)
APP_BUILD_MONTH = $(shell date +%-m)
APP_BUILD_DAY = $(shell date +%-d)

#####################################################
CC	= gcc
CPP	= g++
LD	= ld
AR	= ar
STRIP = strip
NM	= nm
RANLIB = ranlib
ARFLAG	= qcs

#####################################################
STRIPFLAGS = -g --strip-unneeded
#####################################################

GSTREAMER_INC_PATH = -I$(BUILD_HOME_PATH)/usr/include/gstreamer-1.0

GSTREAMER_INC_PATH += -I$(BUILD_HOME_PATH)/usr/include/glib-2.0

GSTREAMER_INC_PATH += -I$(BUILD_HOME_PATH)/usr/lib/x86_64-linux-gnu/glib-2.0/include

GSTREAMER_LIB_PATH = $(BUILD_HOME_PATH)/usr/lib/gstreamer-1.0

GSTREMER_PKG    = gstreamer-1.0 gstreamer-app-1.0 glib-2.0 gobject-2.0 gstreamer-rtsp-server-1.0

GSTREAMER_CFG   = `pkg-config --cflags --libs $(GSTREMER_PKG)`

GSTREAMER_LIB   = `pkg-config --libs $(GSTREMER_PKG)`


LIBS = -Wl,-rpath,$(GSTREAMER_LIB_PATH) $(GSTREAMER_CFG) -lgstapp-1.0

LIBDIR = -L$(APP_LIB_PATH)

LIBDIR += -L$(GSTREAMER_LIB_PATH)

LFLAGS = $(LIBDIR) $(LIBS)

 
CFLAGS = -g -Wall -O2

CFLAGS += -I. -I.. -I$(APP_INC_PATH) 

CFLAGS += $(GSTREAMER_INC_PATH)

CFLAGS += -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE

CFLAGS += -fno-strict-aliasing -Wno-unused-function -Wno-noexcept-type -Wparentheses

CFLAGS += -Wno-stringop-truncation -Wno-narrowing -Wno-pointer-arith
######################################################
export APP_DIR
export SYSTEM
export STRIPFLAGS
export LIBDIR
