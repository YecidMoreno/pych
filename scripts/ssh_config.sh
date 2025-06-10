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

mkdir -p  "${HH_CORE_WORK}/ssh"
cd ${HH_CORE_WORK}/ssh

ssh-keygen -t rsa -b 4096 -C "hh_core" -f id_rsa

ssh ${REMOTE_SSH} "mkdir -p ~/.ssh && chmod 700 ~/.ssh"
# cat ${REMOTE_SSH_KEY}.pub | ssh ${REMOTE_SSH} "cat >> ~/.ssh/authorized_keys && chmod 600 ~/.ssh/authorized_keys"

ssh-copy-id -i  ${REMOTE_SSH_KEY}.pub ${REMOTE_SSH}

