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

PARENT_DIR=${HH_CORE_WORK}

send_over_rsync() {

    SSH_CMD="ssh -i ${REMOTE_SSH_KEY}"

    rsync -avz -e "${SSH_CMD}" $PARENT_DIR/release/${REMOTE_ARCH}/ ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK} &&
        rsync -avz -e "${SSH_CMD}" "$PARENT_DIR/test/robot.json" ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK}/

    rsync -avz -e "${SSH_CMD}" $PARENT_DIR/*.sh ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK} &&
        rsync -avz -e "${SSH_CMD}" $PARENT_DIR/scripts ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK}
}

send_over_sshfs(){
    rsync -avz  $PARENT_DIR/release/${REMOTE_ARCH}/ ${REMOTE_LOCAL}/ &&
        rsync -avz  "$PARENT_DIR/test/robot.json" ${REMOTE_LOCAL}/

    rsync -avz  $PARENT_DIR/*.sh ${REMOTE_LOCAL} &&
        rsync -avz  $PARENT_DIR/scripts ${REMOTE_LOCAL}
}

if findmnt -rno TARGET "${REMOTE_LOCAL}" >/dev/null; then
    echo "La carpeta '$RUTA' está montada: using send_over_sshfs"
    send_over_sshfs
else
    echo "La carpeta '$RUTA' NO está montada: using send_over_rsync"
    send_over_rsync
fi
