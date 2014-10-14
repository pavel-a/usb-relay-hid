# Makefile for HID USB relay utility, usbrelay-cmd
# pa01 16-apr-2014
#
# prototype:
# Author: Christian Starkjohann
# Creation Date: 2008-04-11
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)


# For Unix/Linux, Mac OS X:
USBFLAGS=   `libusb-config --cflags`
USBLIBS=    `libusb-config --libs`
EXE_SUFFIX=

# Use the following 3 lines on Windows and comment out the 3 above:
#USBFLAGS=
#USBLIBS=    -lhid -lusb -lsetupapi
#EXE_SUFFIX= .exe

#+pa GCC on my ubuntu won't pick this dir by default ?!
#USBFLAGS+=-I/usr/include

CC=		gcc
CFLAGS=		-O -Wall $(USBFLAGS)
LIBS=		$(USBLIBS)

OBJ=		usbrelay-cmd.o hiddata.o
PROGRAM=	usbrelay-cmd$(EXE_SUFFIX)

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS)

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM)

.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -c $*.c -o $*.o
