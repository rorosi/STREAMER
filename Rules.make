####################################################
# Build configuration
# Usage:
#   make                        # native x86
#   make ARCH=aarch64           # cross-compile for ARM64
#   make ARCH=aarch64 SYSROOT=/opt/sysroot/aarch64
####################################################

# Target architecture (x86 | aarch64)
ARCH ?= x86

BUILD_HOME := /home/server/
APP_DIR    := $(BUILD_HOME)/streamer

LIB_DIR = $(APP_DIR)/lib/$(ARCH)
BIN_DIR = $(APP_DIR)/bin/$(ARCH)

####################################################
# Toolchain
####################################################
ifeq ($(ARCH), aarch64)
  CROSS_COMPILE := aarch64-linux-gnu-
else
  CROSS_COMPILE :=
endif

CC     = $(CROSS_COMPILE)gcc
CPP    = $(CROSS_COMPILE)g++
LD     = $(CROSS_COMPILE)ld
AR     = $(CROSS_COMPILE)ar
STRIP  = $(CROSS_COMPILE)strip
NM     = $(CROSS_COMPILE)nm
RANLIB = $(CROSS_COMPILE)ranlib

####################################################
# GStreamer / pkg-config
####################################################
GSTREMER_PKG = gstreamer-1.0 gstreamer-app-1.0 glib-2.0 gobject-2.0 gstreamer-rtsp-server-1.0

ifeq ($(ARCH), aarch64)
  # Sysroot: override with make SYSROOT=/path/to/sysroot
  SYSROOT ?= /opt/sysroot/aarch64

  PKG_CONFIG = PKG_CONFIG_SYSROOT_DIR=$(SYSROOT) \
               PKG_CONFIG_PATH=$(SYSROOT)/usr/lib/pkgconfig:$(SYSROOT)/usr/lib/aarch64-linux-gnu/pkgconfig \
               pkg-config

  GSTREAMER_INC_PATH  = -I$(SYSROOT)/usr/include/gstreamer-1.0
  GSTREAMER_INC_PATH += -I$(SYSROOT)/usr/include/glib-2.0
  GSTREAMER_INC_PATH += -I$(SYSROOT)/usr/lib/aarch64-linux-gnu/glib-2.0/include
  GSTREAMER_LIB_PATH  = $(SYSROOT)/usr/lib/gstreamer-1.0
else
  PKG_CONFIG = pkg-config

  GSTREAMER_INC_PATH  = $(shell $(PKG_CONFIG) --cflags-only-I $(GSTREMER_PKG))
  GSTREAMER_LIB_PATH  = /usr/lib/x86_64-linux-gnu/gstreamer-1.0
endif

GSTREAMER_CFG = $(shell $(PKG_CONFIG) --cflags --libs $(GSTREMER_PKG))

####################################################
# Flags
####################################################
APP_ROOT_PATH = $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)
APP_LIB_PATH  = $(APP_ROOT_PATH)/../lib/$(ARCH)
APP_INC_PATH  = $(APP_ROOT_PATH)/../include

LIBDIR  = -L$(APP_LIB_PATH)
LIBDIR += -L$(GSTREAMER_LIB_PATH)

LIBS  = -Wl,-rpath,$(GSTREAMER_LIB_PATH) $(GSTREAMER_CFG) -lgstapp-1.0

CFLAGS  = -g -Wall -O2
CFLAGS += -I. -I.. -I$(APP_INC_PATH)
CFLAGS += $(GSTREAMER_INC_PATH)
CFLAGS += -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE
CFLAGS += -fno-strict-aliasing -Wno-unused-function -Wno-noexcept-type -Wparentheses
CFLAGS += -Wno-stringop-truncation -Wno-narrowing -Wno-pointer-arith

####################################################
export APP_DIR
export ARCH
export LIBDIR
