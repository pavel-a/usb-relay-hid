#!/bin/bash
# Make release, OS X binaries, universal
# Run this after build

# Release tree:
# bin-OSX/
# doc/
# usb_relay_device_lib_devel/
#   OS X framework ??
# Test/

echo "TO DO!" && exit 1

set -e

export REL=_rel

mkdirr() {
 if [ ! -d $1 ]; then mkdir -r $1 ; fi 
}

mkdirr $REL

copy_bin_osx() {
mkdirr $1
cp commandline/makeosx/hidusb-relay-cmd $1
cp commandline/makeosx/usb_relay_device.dylib $1
}

copy_bin_osx $REL/bin-OSX

copy_noarch() {
mkdirr $REL/doc
cp doc/Readme_USB-Relay-DLL.md  $REL/doc/
cp commandline/README-hidusb-relay-cmd.txt $REL/doc/

mkdirr $REL/usb_relay_device_lib_devel
cp lib/usb_relay_device.h  $REL/usb_relay_device_lib_devel
}

copy_noarch

# Tests ...
mkdirr  $REL/Test
cp commandline/relay1-pulse.sh $REL/Test

echo "Done"
