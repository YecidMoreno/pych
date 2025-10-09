#!/bin/bash

source ./activate.sh

EXEC="."
FILE="models/ExoTaov1.json"
FILE="models/ModeCurrent_EPOS2.json"
# FILE="models/ModeVelocity_EPOS2.json"

export LD_LIBRARY_PATH="$EXEC/lib"

eval "$EXEC/bin/testCore -f $FILE"