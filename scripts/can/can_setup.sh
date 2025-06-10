#!/bin/bash
/sbin/ip link set can0 down
/sbin/ip link set can0 up type can bitrate 1000000
/sbin/ifconfig can0 txqueuelen 1000
/sbin/ifconfig can0 up

# /usr/local/bin/can_setup.sh
#sudo chmod +x /usr/local/bin/can_setup.sh

# /etc/udev/rules.d/99-canusb.rules
# lsusb idVendor:idProduct
#SUBSYSTEM=="usb", ATTRS{idVendor}=="1d50", ATTRS{idProduct}=="606f", RUN+="/usr/local/bin/can_setup.sh"
