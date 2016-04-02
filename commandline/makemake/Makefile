# Makefile for HID USB relay utility, hidusbrelay-cmd and library
# for Linux
# (Quick'n'dirty - no .h dependencies, etc.)
# Assume make is run in this dir
# pa05 21-oct-2015

# prototype:
# Author: Christian Starkjohann
# Creation Date: 2008-04-11
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH

SRCDIR=..
VPATH = $(SRCDIR)

CMD_UTILITY := hidusb-relay-cmd
SHRLIB      := usb_relay_device.so

# For Linux: Using old simple version 0.1 of libusb
HIDDATA  := hiddata_libusb01
USBFLAGS := $(shell libusb-config --cflags)
USBLIBS  := $(shell libusb-config --libs)
EXE_SUFFIX=


# Use the following lines to build for Windows and comment out the 3 above:
#HIDDATA=hiddata_mswin
#USBFLAGS=
#USBLIBS=    -lhid -lsetupapi
#EXE_SUFFIX= .exe

#+pa GCC on my ubuntu 12 won't pick this dir by default ?!
#USBFLAGS+=-I/usr/include

SRCS = usbrelay-cmd $(HIDDATA)

PROGRAM=	$(CMD_UTILITY)$(EXE_SUFFIX)

all: $(PROGRAM) $(SHRLIB)

CC=		gcc
DEBUGFLAGS=
CFLAGS=		-O -Wall $(USBFLAGS) $(DEBUGFLAGS) -I$(SRCDIR)
LIBS=		$(USBLIBS)
OBJ=		$(addsuffix .o,$(SRCS))


$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS)

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f *.o $(PROGRAM) $(SHRLIB)

%.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -c $*.c -o $*.o

#
# Shared library
# The main lib source is in lib/ and hiddata src with the cmd utility - reorg!
#
SRCS_LIB = $(HIDDATA) usb_relay_lib
CFLAGS += -I../../lib -fPIC
VPATH =../../lib:..

$(SHRLIB):	$(addsuffix .o, $(SRCS_LIB))
	$(CC) -shared -Wl,-shared $(addsuffix .o, $(SRCS_LIB)) $(LIBS) -o $(SHRLIB)


