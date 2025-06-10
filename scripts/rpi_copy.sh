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

PARENT_DIR=${HH_CORE_WORK}

rsync -avz $PARENT_DIR/release/${REMOTE_ARCH} ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK}/ &&
    rsync -avz "$PARENT_DIR/robot.json" ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK}/

rsync -avz $PARENT_DIR/*.sh ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK} &&
    rsync -avz $PARENT_DIR/scripts ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK}
