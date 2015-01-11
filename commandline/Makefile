# Makefile for HID USB relay utility, hidusbrelay-cmd
# pa02 22-apr-2014
#
# prototype:
# Author: Christian Starkjohann
# Creation Date: 2008-04-11
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)


CMD_UTILITY=hidusb-relay-cmd

# For Unix/Linux :
# Using old simple version 0.1 of libusb
USBFLAGS=   `libusb-config --cflags`
USBLIBS=    `libusb-config --libs`
EXE_SUFFIX=

# Use the following 3 lines on Windows and comment out the 3 above:
#USBFLAGS=
#USBLIBS=    -lhid -lsetupapi
#EXE_SUFFIX= .exe

#+pa GCC on my ubuntu 12 won't pick this dir by default ?!
#USBFLAGS+=-I/usr/include

HIDDATA=hiddata_libusb01

CC=		gcc
DEBUGFLAGS=
CFLAGS=		-O -Wall $(USBFLAGS) $(DEBUGFLAGS)
LIBS=		$(USBLIBS)

OBJ=		usbrelay-cmd.o $(HIDDATA).o
PROGRAM=	$(CMD_UTILITY)$(EXE_SUFFIX)

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS)

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM)

.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -c $*.c -o $*.o
