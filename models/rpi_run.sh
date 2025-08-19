#!/bin/bash

source ./activate.sh

EXEC="."
FILE="models/ScienceMode3.json"

export LD_LIBRARY_PATH="$EXEC/lib"

eval "$EXEC/bin/testCore -f $FILE"