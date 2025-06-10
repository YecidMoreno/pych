#!/bin/bash
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"

export TERM=xterm-256color

export HH_CORE_ACTIVATED="1"
export HH_CORE_WORK="$(pwd)"

export REMOTE_SSH_KEY="${HH_CORE_WORK}/ssh/id_rsa"

export REMOTE_SSH_ARGS="-i ${REMOTE_SSH_KEY}"

export REMOTE_ADDR="192.168.0.101"
export REMOTE_USER="pi"
export REMOTE_SSH="${REMOTE_USER}@${REMOTE_ADDR}"
export REMOTE_WORK="/home/pi"
export REMOTE_ARCH="aarch64"
export PS1="(core)\[\e[1;32m\]\u@\h:\[\e[1;34m\]\w\[\e[0m\]\$ "