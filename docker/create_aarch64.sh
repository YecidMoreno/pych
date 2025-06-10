#!/usr/bin/env bash

docker run --rm dockcross/linux-arm64 > docker/aarch64.xdockcross
chmod +x docker/aarch64.xdockcross

# docker run --rm dockcross/linux-armv7:20250527-abab9c6 > dockcross-linux
