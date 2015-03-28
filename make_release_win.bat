@echo off
: Make release, Windows binaries
: Run this after binaries are built

: Release tree:
: bin-Win32/
: bin-Win64/
: doc/
: Test/
: usb_relay_device_lib_devel/
:    Win32, Win64 ...
: bin-Linux-x86/
: bin-Linux-x64/
: bin-OSX/

set REL=_rel
if not exist _rel mkdir _rel
if not exist %REL%\bin-Win32 mkdir %REL%\bin-Win32
if not exist %REL%\bin-Win64 mkdir %REL%\bin-Win64

@echo Copying x86 binaries...
copy Windows-VS\Release\USB_RELAY_DEVICE.dll %REL%\bin-Win32
: ... vp
copy Windows-VS\Release\hidusb-relay-cmd.exe %REL%\bin-Win32
: ... vp
copy Windows-VS\Release\GUIapp.exe %REL%\bin-Win32
: ... vp

if not exist Windows-VS\x64\Release\* goto L64_1
@echo Copying x64 binaries...
copy Windows-VS\x64\Release\USB_RELAY_DEVICE.dll %REL%\bin-Win64
: ... vp
copy Windows-VS\x64\Release\hidusb-relay-cmd.exe %REL%\bin-Win64
: ... vp
copy Windows-VS\x64\Release\GUIapp.exe %REL%\bin-Win64
: ... vp
:L64_1

@echo Copying docum ...
if not exist %REL%\doc mkdir %REL%\doc
copy doc\Readme_USB-Relay-DLL.md  %REL%\doc
copy commandline\README-hidusb-relay-cmd.txt %REL%\doc

@echo Copying developer stuff ...
if not exist %REL%\usb_relay_device_lib_devel mkdir %REL%\usb_relay_device_lib_devel
mkdir %REL%\usb_relay_device_lib_devel\Win32
mkdir %REL%\usb_relay_device_lib_devel\Win64
copy lib\usb_relay_device.h  %REL%\usb_relay_device_lib_devel
copy Windows-VS\Release\USB_RELAY_DEVICE.lib  %REL%\usb_relay_device_lib_devel\Win32
copy Windows-VS\x64\Release\USB_RELAY_DEVICE.lib  %REL%\usb_relay_device_lib_devel\Win64

: ?? Copy also PDBs? debug binaries?

: Test ...

@echo Done.
