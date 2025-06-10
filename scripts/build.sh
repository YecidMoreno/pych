#!/bin/bash
SCRIPT_DIR="$(dirname "$(readlink -f "$0")")"
PARENT_DIR="$(dirname "$SCRIPT_DIR")"

cd $PARENT_DIR

ARCH=${1:-$(uname -m)}
NJOURNAL=${2:-$(nproc)}

if ldd --version 2>&1 | grep -qi musl; then
    PLATFORM="_musl"
elif [ "$ARCH" = "x86_64" ]; then
    PLATFORM="_amd64"
else
    PLATFORM=""
fi

TARGET="${ARCH}${PLATFORM}"
TARGET="${TARGET,,}"

CMD="cmake -B build/$TARGET -S . -DTARGET_PWD=$TARGET && cmake --build build/$TARGET -j$NJOURNAL"
echo "Building for: $TARGET"
echo $CMD


if [ -z "$1" ]; then
    bash -c "$CMD"
    exit 0
fi

if [ "$ARCH" == "aarch64" ]; then
    ./docker/aarch64.xdockcross bash -c "$CMD"
elif [ "$TARGET" == "x86_64_musl" ]; then
    ./docker/x86_64_musl.xdockcross bash -c "$CMD"
else
    echo "not defined build for: $TARGET"
    exit -1
fi
