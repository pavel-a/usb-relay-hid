
The HID Relay device details:
=========================

Two USB HID relays are on eBay
(search for "5V USB Relay Programmable Computer Control For Smart Home"):
 one- and two-channel models; there can be also a four-channel model (?)

- Each relay can connect up to 250V/10A AC or 28V DC load. Please see the electrical specs provided by vendors. 
- USB-powered only; no external power input. When disconnected from host, the relays state is not remembered.
- The board has red status LEDs for each channels and a green power LED.
- Each relay channel has Normally Open and Normally Closed contact. The status LED lighs when the Normally Open contacts get closed. We'll name this state "**ON**", and the "normal" state - "**OFF**".
- The USB interface is low-speed (USB 1.1) HID, based on [V-USB](http://www.obdev.at/products/vusb/) software USB emulation library.
- The USB device ID is VEN=16C0 DEV=05DF. This is the shared ID for V-USB HID class devices. The V-USB documentation describes how to detect the real vendor and product IDs for such devices.
- For 1 and 2-channel boards, the vendor ID is "www.dcttech.com" and product IDs are "USBRelay1", "USBRelay2".
- Each board has an unique 5-character ID string in its HID descriptor (not to be confused with the standard USB "serial number"). We don't know whether these IDs are really unique, at least, on all our devices they are different.
