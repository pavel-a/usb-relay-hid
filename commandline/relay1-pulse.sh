#!/bin/sh
# Pulse USB relay channel for  (sec) seconds
# Needs root acccess! run with sudo!
sec=${1}
chan=1
SUDO=sudo
$SUDO ./usbrelay-cmd ON $chan
sleep $sec
$SUDO ./usbrelay-cmd OFF $chan
