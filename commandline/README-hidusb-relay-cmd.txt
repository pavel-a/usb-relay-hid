Command line utility for Chinese USB HID relays
(USB 1.1 HID devices, VEN=5824 (0x16c0) DEV=1503 (0x05DF), based on the V-USB project)

Rev. 1.0 (16-apr-2014)

Usage:
  usbrelay-cmd ON|OFF <num>   -- turn the relay ON or OFF
  usbrelay-cmd STATE          -- print state of relays and the "unique ID"
  
  where <num> is the relay number: 1-2 or "*" for all

Each relay has two contact pairs: Normally Open and Normally Closed.
The OFF state is the "normal" state, when the red LED is off, and the Normally Open contacts are disconnected.
The ON state is when the red LED is on, and the Normally Open contacts are connected.

Note: In the original s/w readme, "Open" means ON, "Close" means OFF.
  
Currently tested only with the 2-relay device.
Finds the FIRST AVAILABLE RELAY device! Does not address by the "unique ID" yet!

SOURCE based on a V-USB HID example.
LICENSE: TBD!
         Should be non-GPL; rewrite all prototype code!


For Windows XP and later:
 - using the in-box hid.dll and WDK 7.1.
 - build with VC++ 2008

For Linux (PC, x86):
 - using the old libusb (v 0.1; usb.h)
 - tested on Centos 5, Ubuntu 12.04
 - requires root!
   To grant access to users, define udev rule for this device (or all HID). Example:
   SUBSYSTEMS=="usb", ATTRS{idVendor}=="16c0", ATTRS{idProduct}=="05df", MODE:="0666" 
 
>>> Target for 1st public release:
================================
1. Remove GPL encumberment
2. Make a .so for Linux (Windows already has orig DLL)

Other TODO's
============
TODO: support multiple devices, support one-relay and other existing variants
  usbrelay-cmd -SN=XXXXX ON|OFF <num>
  usbrelay-cmd -SN=XXXXX  STATE       
    or -s XXXXX or --SN or -ID ...
    
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
productName = "USBRelay2"

