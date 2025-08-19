#!/bin/bash
# SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

SOURCE="${BASH_SOURCE[0]}"
while [ -h "$SOURCE" ]; do
  DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
  SOURCE="$(readlink "$SOURCE")"
  [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"
done
SCRIPT_DIR="$(cd -P "$(dirname "$SOURCE")" >/dev/null 2>&1 && pwd)"
SCRIPT_DIR="/home/inception/git/pych"

# export TERM=xterm-256color

export PYCH_CORE_EXEC="bin/testCore"
export PYCH_CORE_EXEC_ARG="-f models/ScienceMode3.json"

export PYCH_CORE_ACTIVATED="1"
# export PYCH_CORE_WORK="$(pwd)"
export PYCH_CORE_WORK=$SCRIPT_DIR

export REMOTE_SSH_KEY="${PYCH_CORE_WORK}/ssh/id_rsa"
export REMOTE_SSH_ARGS="-i ${REMOTE_SSH_KEY} "

# export REMOTE_ADDR="192.168.0.101"
export REMOTE_ADDR="raspberrypi.local"
export REMOTE_USER="pi"
export REMOTE_WORK="/home/pi/core"
export REMOTE_ARCH="aarch64-unknown-linux-gnu"

export REMOTE_SSH="${REMOTE_USER}@${REMOTE_ADDR}"
export REMOTE_LOCAL="${PYCH_CORE_WORK}/remote"
export REMOTE_TTY="ssh $REMOTE_SSH_ARGS $REMOTE_SSH"

export PYCH_NAME="core"
export PS1='($(echo $PYCH_NAME))\[\e[1;32m\]\u@\h:\[\e[1;34m\]\w\[\e[0m\]\$ '

# source ${SCRIPT_DIR}/scripts/build-completion.sh

export LD_LIBRARY_PATH=lib/:${LD_LIBRARY_PATH}

echo "PYCH - Activated in ($(pwd))"