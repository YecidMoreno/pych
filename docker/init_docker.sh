#!/usr/bin/env bash

docker build -t alpine-pych-x86_64:v0.1 -f docker/x86_64_musl.Dockerfile .

docker pull dockcross/linux-arm64:20250109-7bf589c
docker pull dockcross/linux-x86_64-full:20250611-3deaae3

# docker run --rm dockcross/linux-arm64:20250109-7bf589c > docker/aarch64.xdockcross
# chmod +x docker/aarch64.xdockcross

# docker run --rm dockcross/linux-x86_64-full:20250611-3deaae3 > docker/x86_64_amd64.xdockcross
# chmod +x docker/x86_64_amd64.xdockcross