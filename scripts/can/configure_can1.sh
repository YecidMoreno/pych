#!/bin/bash

# Ensure the script is run as root
if [ "$EUID" -ne 0 ]; then
  echo "Please run this script as root (using sudo)."
  exit 1
fi

CONFIG_FILE="/boot/firmware/config.txt"

# 1. Create a backup just in case
cp "$CONFIG_FILE" "${CONFIG_FILE}.bak"
echo "Backup created at ${CONFIG_FILE}.bak"

sudo raspi-config nonint do_spi 0

# 2. Define the lines to add
LINE2="dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=25"

echo "Configuring CAN bus in $CONFIG_FILE..."

# Add LINE2 if it doesn't exist
if ! grep -qF "$LINE2" "$CONFIG_FILE"; then
    echo "$LINE2" >> "$CONFIG_FILE"
    echo "Added: $LINE2"
else
    echo "Already exists: $LINE2"
fi

echo "Configuration completed successfully!"
echo "Remember to reboot your Raspberry Pi to apply the changes."

sudo cp 70-can.rules /etc/udev/rules.d/