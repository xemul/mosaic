CFLAGS = -Wall -Werror
CC = gcc

# Installation directories
BINDIR=/usr/bin
LIBDIR=/usr/lib
# For DEB-based systems
DEB_HOST_MULTIARCH ?= $(shell dpkg-architecture -qDEB_HOST_MULTIARCH 2>/dev/null)
ifneq "$(DEB_HOST_MULTIARCH)" ""
LIBDIR=/usr/lib/$(DEB_HOST_MULTIARCH)
# For RPM-based systems
else ifeq "$(shell uname -m)" "x86_64"
LIBDIR=/usr/lib64
endif
