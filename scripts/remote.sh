#!/bin/bash

if [ -n "$PYCH_CORE_ACTIVATED" ] &&
    [ -n "$REMOTE_ADDR" ] &&
    [ -n "$REMOTE_USER" ] &&
    [ -n "$REMOTE_WORK" ] &&
    [ -n "$REMOTE_ARCH" ]; then
    echo "Sending to: ${REMOTE_USER}@${REMOTE_ADDR}:${REMOTE_WORK}"
else
    echo "exit 0"
    exit 0
fi

PARENT_DIR=${PYCH_CORE_WORK}
SEND_FILES="$PARENT_DIR/release/${REMOTE_ARCH}/bin \
    $PARENT_DIR/release/${REMOTE_ARCH}/lib \
    $PARENT_DIR/release/${REMOTE_ARCH}/plugins \
    $PARENT_DIR/models \
    $PARENT_DIR/*.sh \
    $PARENT_DIR/scripts \
    /home/inception/git/pych_plugins/release/${REMOTE_ARCH}/plugins"

remote_build() {
    pych-xbuild $REMOTE_ARCH
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

        mkdir -p ${PYCH_CORE_WORK}/remote

        CMD="sshfs -o IdentityFile=${REMOTE_SSH_KEY} ${REMOTE_SSH}:${REMOTE_WORK} ${PYCH_CORE_WORK}/remote"
        echo $CMD
        bash -c "$CMD"
    fi
}

umount_sshfs() {
    if findmnt -rno TARGET "${REMOTE_LOCAL}" >/dev/null; then
        echo "Desmontando carpeta remota"
        fusermount -u ${PYCH_CORE_WORK}/remote
    fi
}

remote_mount() {
    if findmnt -rno TARGET "${REMOTE_LOCAL}" >/dev/null; then
        umount_sshfs && sleep 1 && mount_sshfs
    else
        mount_sshfs
    fi

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
