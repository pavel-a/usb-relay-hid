# Makefile for HID USB relay utility, hidusbrelay-cmd
# pa03 24-jan-2015
#
# prototype:
# Author: Christian Starkjohann
# Creation Date: 2008-04-11
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

SRCDIR=..
VPATH = $(SRCDIR)

CMD_UTILITY=hidusb-relay-cmd

# For Linux
# Using old simple version 0.1 of libusb
HIDDATA=hiddata_libusb01
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

CC=		gcc
DEBUGFLAGS=
CFLAGS=		-O -Wall $(USBFLAGS) $(DEBUGFLAGS) -I$(SRCDIR)
LIBS=		$(USBLIBS)
OBJ=		$(addsuffix .o,$(SRCS))

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS)

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM)

%.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -c $*.c -o $*.o
