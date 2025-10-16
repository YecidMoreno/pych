#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

TO_BUILD_DIR=${1:-"$SCRIPT_DIR"}
TARGET=${2:-"host"}
shift 2

cp $PYCH_CORE_WORK/cmake/external_build/CMakeLists.txt "$TO_BUILD_DIR"

pych-xbuild $TARGET  --src "$TO_BUILD_DIR" -v $PYCH_CORE_WORK/release/'$TARGET':/core $@ \
&& \
rsync -avz \
    $TO_BUILD_DIR/release \
    $PYCH_CORE_WORK

rm $TO_BUILD_DIR/CMakeLists.txt

# pych-xbuild $TARGET --src "$TO_BUILD_DIR" -v $PYCH_CORE_WORK/release/'$TARGET':/core $@  -ps "ls"

# TARGET="none"
# --cmake-pwd "/core/lib/cmake/pych_core/external_build"
# pych-xbuild $TARGET   --src "$TO_BUILD_DIR"   -v $PYCH_CORE_WORK/release/'$TARGET':/core $@ 