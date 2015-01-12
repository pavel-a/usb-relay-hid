Command line utility for Chinese USB HID relays
(USB 1.1 HID devices, VEN=5824 (0x16c0) DEV=1503 (0x05DF), based on the V-USB project)

Rev. 1.2 (22-Nov-2014)

Usage:
  usbrelay-cmd [ID=XXXXX] ON|OFF <num>   -- turn the relay ON or OFF
  
      where <num> is the relay number: 1-2 or "ALL"

  usbrelay-cmd STATUS         -- print state of one relay device with its "unique ID"
  usbrelay-cmd ENUM           -- print state of all devices and their "unique IDs"
  
Parameter ID=XXXXX  specifies one relay device if several are connected.
Without the ID= parameter, if several devices are connected, the program uses any one of them.
The ID string is case sensitive.

Examples:

  usbrelay-cmd id=ABCDE on 1
  usbrelay-cmd id=ABCDE OFF ALL
  usbrelay-cmd id=ABCDE STATUS

  usbrelay-cmd ON ALL  -- uses first found relay device
  usbrelay-cmd STATUS  -- uses first found relay device
  

REMARKS

The Enum command lists the IDs of all available relay devices.
The ID strings are case sensitive and should be exactly as output by the "enum" command.

Each relay has two contact pairs: Normally Open and Normally Closed.
The OFF state is the "normal" state, when the red LED is off, and the Normally Open contacts are disconnected.
The ON state is when the red LED is on, and the Normally Open contacts are connected.

Note: In the original s/w readme, "Open" means ON, "Close" means OFF.

This program has been tested with 1- and 2-relay devices.

On Linux, this program requires root access by default!
To grant access to non-root users, define udev rule for the devices:
Example:
   SUBSYSTEMS=="usb", ATTRS{idVendor}=="16c0", ATTRS{idProduct}=="05df", MODE:="0666" 


BUILDING

For Windows XP and newer:
 - Build with VC++ 2008 with WDK 7.1, or VC++ 2013 "community edition" with WDK 8.1
 
For Linux (PC, x86):
 - using the old libusb (v. 0.1; usb.h)
 - tested on Centos 5, Ubuntu 12.04, Mint

For OS X 10.9:
 - build with Xcode and IOKit framewrork
 
~~
