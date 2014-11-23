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

For Windows XP and later:
 - Using the in-box hid.dll
 - Build with VC++ 2008 and WDK 7.1, or newer VC++ with the suitable WDK
 
For Linux (PC, x86):
 - using the old libusb (v. 0.1; usb.h)
 - tested on Centos 5, Ubuntu 12.04, Mint

SOURCE based on a V-USB HID example.
LICENSE: TBD!
         Should be non-GPL; rewrite all prototype code!

>>> Target for 1st public release:
================================
1. Remove GPL encumberment
2. Make a .so for Linux (Windows already has orig DLL)

Other TODO's
============
    
 *** Check that these devices indeed have unique IDs!
  The orig. h file mentions  function:   usb_relay_device_set_serial(int hHandle, char serial[5]);
  which may be used by mfg to burn the ID? but the orig. DLL does not export it.
  In detection code (find by "unique ID"), check only by product name, Ignore vendor string?

TODO: Make a .so callable from Java, Python etc.
      For this, move to libusb 1.x (libusb.h)? 
TODO: provide the original API "usb_relay_device.h" API, compatible with the orig. usb_relay_device.dll
      The orig .h file and example are in orig dir (slightly adapted and updated to VC2008)  
     In orig .h file relay state "open" means ON (the red LED is on). "Close" means OFF (the red LED is off)
~~

Uses code by:
Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH 
Author: Christian Starkjohann
Creation Date: 2008-04-11

~~
The relay device ID - from HID attributes:
vendorName = "www.dcttech.com" -> this site does not exist (squatted)
productName = "USBRelayN" where N=1|2|4

