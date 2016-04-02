#!/bin/sh
# Pulse USB relay channel for  (sec) seconds
# If access to the USB device requires root, run with sudo
sec=${1:-1}
chan=1
SUDO=sudo
dir="./"

$SUDO ${dir}hidusb-relay-cmd ON $chan
sleep $sec
$SUDO ${dir}hidusb-relay-cmd OFF $chan
