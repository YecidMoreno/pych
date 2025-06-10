#!/usr/bin/env bash

docker build -t alpine-can-dev .

docker run --rm -it \
  --network=host \
  --cap-add=NET_ADMIN \
  -v ~/git/commIO/libs/rapidjson/include/rapidjson:/usr/local/include/rapidjson \
  -v ~/git/commIO:/opt/host \
  -v ~/git/commIO:/work \
  alpine-can-dev bash "$@"

  # --device=/dev/ttyUSB0 \