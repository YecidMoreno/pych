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



mkdir -p ${HH_CORE_WORK}/remote

CMD="sshfs -f -o IdentityFile=${REMOTE_SSH_KEY} ${REMOTE_SSH}:${REMOTE_WORK} ${HH_CORE_WORK}/remote"
echo $CMD
echo "Presiona Ctrl+C para finalizar ..."

bash -c "$CMD"
fusermount -u ${HH_CORE_WORK}/remote


