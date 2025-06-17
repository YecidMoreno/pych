#!/bin/bash

if [ -n "$HH_CORE_ACTIVATED" ] &&
    [ -n "$REMOTE_ADDR" ] &&
    [ -n "$REMOTE_USER" ] &&
    [ -n "$REMOTE_WORK" ] &&
    [ -n "$REMOTE_LOCAL" ] &&
    [ -n "$REMOTE_ARCH" ]; then
    echo "Sending to: ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK}"
else
    exit -1
fi

mount_sshfs() {
    mkdir -p ${HH_CORE_WORK}/remote

    CMD="sshfs -o IdentityFile=${REMOTE_SSH_KEY} ${REMOTE_SSH}:${REMOTE_WORK} ${HH_CORE_WORK}/remote"
    echo $CMD
    bash -c "$CMD"
}

if findmnt -rno TARGET "${REMOTE_LOCAL}" >/dev/null; then
    echo "Desmontando carpeta remota"
    fusermount -u ${HH_CORE_WORK}/remote
    exit -1
else
    echo "Montando carpeta remota"
    mount_sshfs
    exit 1 
fi
