#!/usr/bin/env bash

BIN_DIR="release/x86_64-alpine-linux-musl"
CONFIG_FILE="../../test/robot.json"

CMD_SCRIPT="source activate.sh && cd $BIN_DIR && exec bash -c '$CORE_EXEC -f $CONFIG_FILE'"

docker/run.xdockcross -i alpine-pych-x86_64:v0.1 sh -c "$CMD_SCRIPT"
