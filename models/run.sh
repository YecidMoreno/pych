#!/bin/bash

# source /home/inception/git/pych/activate.sh

EXEC="$PYCH_CORE_WORK/release/x86_64-linux-gnu"
# FILE="$PYCH_CORE_WORK/models/FES_ForceSensor.json"
#FILE="$PYCH_CORE_WORK/models/ScienceMode3.json"
FILE="$PYCH_CORE_WORK/models/ExoTaov1.json"
# FILE="$PYCH_CORE_WORK/models/ScienceMode3Scope.json"
# FILE="$PYCH_CORE_WORK/models/FatigueStimation.json"
# FILE="$PYCH_CORE_WORK/models/TCPMatlab.json"

export LD_LIBRARY_PATH="$EXEC/lib"

eval "$EXEC/bin/testCore -f $FILE"