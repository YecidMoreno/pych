#!/bin/bash

if [ -n "$HH_CORE_ACTIVATED" ] &&
    [ -n "$REMOTE_ADDR" ] &&
    [ -n "$REMOTE_USER" ] &&
    [ -n "$REMOTE_WORK" ] &&
    [ -n "$REMOTE_ARCH" ]; then
    echo "Sending to: ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK}"
else
    exit -1
fi

cd $HH_CORE_WORK

./scripts/build.sh $REMOTE_ARCH && 
./scripts/rpi_copy.sh