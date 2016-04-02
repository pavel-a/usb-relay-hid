#!/bin/bash
# Make release, Linux binaries
# Run this after build
# TODO detect x86/x64 build mode ??

# Release tree:
# bin-Linux-x86/
# bin-Linux-x64/
# doc/
# usb_relay_device_lib_devel/
#   (nothing for linux?)
# Test/

set -e

export REL=_rel

mkdirr() {
 if [ ! -d $1 ]; then mkdir --parents $1 ; fi 
}

mkdirr $REL

copy_x86() {
mkdirr $1
cp commandline/makemake/hidusb-relay-cmd $1
cp commandline/makemake/usb_relay_device.so $1
}

copy_x64() {
mkdirr $1
cp commandline/makemake/hidusb-relay-cmd $1
cp commandline/makemake/usb_relay_device.so $1
}

copy_x86 $REL/bin-Linux-x86
# copy_x64 $REL/bin-Linux-x64 

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
cp Test/relay1-pulse.sh $REL/Test
cp Test/[Tt]est*.py $REL/Test

echo "Done"
