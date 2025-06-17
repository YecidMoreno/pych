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

# ssh ${REMOTE_USER}@${REMOTE_ADDR} "export LD_LIBRARY_PATH=${REMOTE_WORK}/${REMOTE_ARCH}/lib &&
# ${REMOTE_WORK}/${REMOTE_ARCH}/test/testCore -f ${REMOTE_WORK}/robot.json"

# ssh ${REMOTE_SSH_ARGS} -t ${REMOTE_USER}@${REMOTE_ADDR} "cd ${REMOTE_WORK} && source activate.sh && exec bash --login"
# ${REMOTE_WORK}/${REMOTE_ARCH}/test/testCore

CMD_RUN="./test/testCore -f robot.json"
CMD=$(cat <<EOF
cd ${REMOTE_WORK} && 
source ./activate.sh &&
export LD_LIBRARY_PATH="${REMOTE_WORK}/lib" &&
exec bash --login
EOF
)

# exec bash --login
# ./test/testCore -f robot.json

ssh ${REMOTE_SSH_ARGS} -t  ${REMOTE_USER}@${REMOTE_ADDR} "$CMD"


# exec bash -c "./test/testCore -f robot.json"