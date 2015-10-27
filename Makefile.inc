NAME=mosaic
VERSION=0.0.0
RELEASE=

CC = gcc
CFLAGS = -Wall -Werror -Wextra

# Temporarily hide some minor warnings (to show, use -Wno-error=...)
CFLAGS += -Wno-unused-parameter -Wno-sign-compare
#CFLAGS += -Wno-error=unused-parameter -Wno-error=sign-compare

# Installation directories
BINDIR=/usr/sbin
LIBDIR=/usr/lib
# For DEB-based systems
DEB_HOST_MULTIARCH ?= $(shell dpkg-architecture -qDEB_HOST_MULTIARCH 2>/dev/null)
ifneq "$(DEB_HOST_MULTIARCH)" ""
LIBDIR=/usr/lib/$(DEB_HOST_MULTIARCH)
# For RPM-based systems
else ifeq "$(shell uname -m)" "x86_64"
LIBDIR=/usr/lib64
endif
