usb-relay-hid
=============

![](http://vusb.wdfiles.com/local--files/project:driver-less-usb-relays-hid-interface/relay2.jpg)

The goal of this little project is to provide open-source API for low-cost USB HID relays 
(search on eBay for "5V USB Relay Programmable Computer Control For Smart Home").

The big advantage of HID-based interface is that no kernel drivers and no simulated COM-ports are needed. It is easy to detect and address the connected devices.

Unfortunatey, the vendors currently offer only API library for Windows, and only in binary form.

We are not associated with the manufacturer(s) of these devices or author(s) of the original software offered by eBay sellers.
Our intent is to make this device usable on Linux (and any other OS that supports libusb) as well as on 64-bit Windows.

The main project web page is on the [V-USB wiki](http://vusb.wikidot.com/project:driver-less-usb-relays-hid-interface "driver-less-usb-relays-hid-interface"). This is because the firmware of the device is based on [V-USB](http://www.obdev.at/products/vusb/index.html). V-USB enables amazingly simple and low-cost USB hardware solutions. This repository, however,  contains only the host side software.



License
-------

Currently we reuse some code from other V-USB projects, which is dual-licensed: GPL + commercial. Until we remove this code, we cannot set any other license. 

We could not locate any copyright information in the software package offered by the eBay sellers. No source code was reused from that package, besides of the C API header (the .h file).

Current state
-------------

A simple command-line utility for Linux (x86 or x64) and Windows.
This is enough for our own usage.

- Windows version builds with VC++ and WDK 7.1.
- Linux version builds with gcc and libusb v. 0.1. Tested on RH and Ubuntu.


To do:
-------

 * Reconstruct the API library, which will be binary compatible with the original software package, and minimal decent documentation.
 * Move the Linux variant to newer libusb version (1.x)
 * Make bindings for Python and Java
 * Maybe, reconstruct the GUI utility
 * Maybe, port the Windows variant to GNU (mingw) toolchain
 
The original API library does not handle hot-plug and removal of USB devices. The command-line utility isn't affected by hot plug/removal, but GUI clients of the library will need to re-enumerate the devices or react to PnP events in OS-specific way. Caller must provide locking in case of accessing the library from multiple threads.


Misc.
-----

Related projects:

[USB Relay](https://github.com/johannesk/usb-relay)  by Johannes Krude - complete project with hardware and firmware, based on V-USB, but not HID - so requires a driver.
