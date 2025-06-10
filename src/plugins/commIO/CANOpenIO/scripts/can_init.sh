# https://canable.io/getting-started.html#socketcan-linux

# sudo apt install dfu-util git
# dfu-util -l
# https://canable.io/updater/


# sudo modprobe can
# sudo modprobe can-raw
# sudo modprobe slcan

# sudo slcan_attach -w -n can0 /dev/ttyACM0
# sudo ip link set can0 up type can bitrate 1000000
# ip -details -statistics link show can0

# sudo ifconfig can0 down
# sudo slcand -o -c -s8 -F /dev/ttyACM0 can0
# sudo ifconfig can0 up
# sudo ifconfig can0 txqueuelen 1000
# candump can0

sudo ifconfig can0 down
sudo ip link set can0 up type can bitrate 1000000
sudo ifconfig can0 txqueuelen 1000
sudo ifconfig can0 up
candump can0
