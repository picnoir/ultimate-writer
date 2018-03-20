# writerpi version
VERSION = 0.1

# Customize below to fit your system

# paths
PREFIX = /usr/local
SYSTEMD_PREFIX = /lib/systemd/system

LIBS = -L -lm -lutil -lpthread -lbcm2835

# flags
CPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600
STCFLAGS = $(INCS) $(CPPFLAGS) $(CFLAGS)
STLDFLAGS = $(LIBS) $(LDFLAGS)

# compiler and linker
# CC = c99

