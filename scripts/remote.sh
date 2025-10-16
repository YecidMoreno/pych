#!/bin/bash

if [ -n "$PYCH_CORE_ACTIVATED" ] &&
    [ -n "$ACTIVATE_SH" ] &&
    [ -n "$REMOTE_ADDR" ] &&
    [ -n "$REMOTE_USER" ] &&
    [ -n "$REMOTE_WORK" ] &&
    [ -n "$REMOTE_ARCH" ]; then
    echo "Sending to: ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK}"
else
    echo "exit 1: Please, activate pych first."
    exit 1
fi



prepare_files() {
    # if [ -d /tmp/pych_tmp ]; then
    #     rm /tmp/pych_tmp -rf
    # fi
    
    mkdir -p /tmp/pych_tmp
    mkdir -p /tmp/pych_tmp/models

    # cp $PROJECT_PATH/$ACTIVATE_SH /tmp/pych_tmp/activate.sh
    # cp -r $PROJECT_PATH /tmp/pych_tmp/models

    rsync -avz "$PROJECT_PATH/$ACTIVATE_SH" /tmp/pych_tmp/activate.sh
    rsync -avz "$PROJECT_PATH" /tmp/pych_tmp/models
}

PARENT_DIR=${PYCH_CORE_WORK}
SEND_FILES="$PARENT_DIR/release/${REMOTE_ARCH}/bin \
    $PARENT_DIR/release/${REMOTE_ARCH}/lib \
    $PARENT_DIR/release/${REMOTE_ARCH}/plugins \
    $PARENT_DIR/scripts \
    /tmp/pych_tmp/"

remote_build() {
    pych-xbuild $REMOTE_ARCH
    exit $?
}

send_over_rsync() {
    SSH_CMD="ssh -i ${REMOTE_SSH_KEY}"

    rsync -avz -e "${SSH_CMD}" \
        $SEND_FILES \
        ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK}
}

send_over_sshfs() {
    rsync -avz \
        $SEND_FILES \
        ${REMOTE_LOCAL}
}

remote_copy() {
    prepare_files
    if findmnt -rno TARGET "${REMOTE_LOCAL}" >/dev/null; then
        echo "La carpeta '$REMOTE_LOCAL' está montada: using send_over_sshfs"
        send_over_sshfs
    else
        echo "La carpeta '$REMOTE_LOCAL' NO está montada: using send_over_rsync"
        send_over_rsync
    fi

}

mount_sshfs() {
    if ! findmnt -rno TARGET "${REMOTE_LOCAL}" >/dev/null; then
        echo "Montando carpeta remota"

        mkdir -p ${REMOTE_LOCAL}

        CMD="sshfs -o IdentityFile=${REMOTE_SSH_KEY} -o ConnectTimeout=5 ${REMOTE_SSH}:${REMOTE_WORK} ${REMOTE_LOCAL}"
        echo $CMD
        bash -c "$CMD"
    fi
}

umount_sshfs() {
    if findmnt -rno TARGET "${REMOTE_LOCAL}" >/dev/null; then
        echo "Desmontando carpeta remota"
        fusermount -u ${REMOTE_LOCAL}
    fi
}

remote_mount() {
    if findmnt -rno TARGET "${REMOTE_LOCAL}" >/dev/null; then
        umount_sshfs && sleep 1 && mount_sshfs
    else
        mount_sshfs
    fi

}

is_mounted() {
    if findmnt -rno TARGET "${REMOTE_LOCAL}" >/dev/null; then
        echo "0"
        exit 0
    else
        echo "1"
        exit 1
    fi
}

cmd_stop() {
    $REMOTE_TTY "pkill -f ${REMOTE_PYCH_BIN}"
    exit $?
}

cmd_run() {
    $REMOTE_TTY "cd ${REMOTE_WORK} && source ./activate.sh && bin/${REMOTE_PYCH_BIN} -f models/${PROJECT_NAME}/${MODEL_NAME}"
    exit $?
}

POSITIONAL=()
while (($# > 0)); do
    case "${1}" in
    build)
        remote_build
        shift
        ;;
    copy)
        remote_copy
        shift
        ;;
    mount)
        mount_sshfs
        shift
        ;;
    umount)
        umount_sshfs
        shift
        ;;
    mnt)
        remote_mount
        shift
        ;;
    ismnt)
        is_mounted
        shift
        ;;

    stop)
        cmd_stop
        shift
        ;;

    start | run)
        cmd_run
        shift
        ;;

    all)
        remote_build &&
            sleep 1 &&
            remote_copy

        shift
        ;;

    tty)
        echo "$REMOTE_TTY"

        $REMOTE_TTY
        shift
        ;;

    *)
        POSITIONAL+=("${1}")
        shift
        ;;
    esac
done

set -- "${POSITIONAL[@]}"
