How to use USB_RELAY_DEVICE DLL
===============================

Version 1.0

Changes to the original DLL:
----------------------------
 - Type used for handles changed from int to intptr_t, for 64-bit compatibility.
   This should be binary compatible with existing 32-bit clients. 


Windows, Visual C++ applications
---------------------------------

Include file name: usb_relay_device.h
Library file name: usb_relay_device.lib

Put the following lines in your source file:

   `#include <usb_relay_device.h>`

   `#pragma comment(lib, "usb_relay_device.lib")`

The file usb_relay_device.dll must be installed with your application.
Use either 32-bit or 64-bit DLL, matching your application.
The DLL may require VC++ redistributable runtime library.
   
Using the API:
--------------   

 * `usb_relay_init()`
 Call this before calling other functions

 * `usb_relay_device_enumerate`
 Enumerates all Relay devices plugged into the PC.
 Returns list of usb_relay_device_info structures.
 Caller should free this list by passing it to usb_relay_device_free_enumerate().
 
 * `usb_relay_device_open`
 Opens one Relay board device by its usb_relay_device_info structure,
 obtained from usb_relay_device_enumerate().

 * `usb_relay_device_open_with_serial_number`
 Opens one Relay board device by its serial number string.
 These strings can be obtained from usb_relay_device_enumerate().
  
 * `usb_relay_device_open_one_relay_channel`
  Turns ON one channel of a Relay board.
  Parameters: 
     - device handle, obtained from usb_relay_device_open()
     - Index: integer from 1 to the maximal number of channels on a board.
  
 * `usb_relay_device_open_all_relay_channel`
  Turns ON all channels on a board.
  
 * `usb_relay_device_close_one_relay_channel`
  Turns OFF one channel of a Relay board.
  Parameters: 
     - device handle, obtained from `usb_relay_device_open()`
     - Index: integer from 1 to the maximal number of channels on a board.

 * `usb_relay_device_close_all_relay_channel`
  Turns OFF all channels on a board.
 
 * `usb_relay_device_get_status`
  Get status of all channels on a board as a bit mask.
  The least significant bit in returned value corresponds to channel 1.
  Bit value 1 means the corresponding relay is switched ON, value 0 means the relay is OFF.
  
 * `usb_relay_device_close`
  Closes the Relay device handle opened by `usb_relay_device_open()` or
   `usb_relay_device_open_with_serial_number()`
 
* `usb_relay_exit`
  Finalizes the library

Error handling
---------------
If error occurred, the API functions that return error code return -1;
functions that return handles or pointers return NULL.
Application is responsible to check the returned value.
  
Notes
-------
 * The library does not detect hot plug/unplug of USB devices.
 * The library is not thread-safe. Client application must ensure that 
  only one thread calls the library.

  
Managed .NET applications (C#, VB.NET)
---------------------------------------
TBD

Python (CPython)
-------------------
TBD

GCC or MinGW applications
--------------------------
TBD

