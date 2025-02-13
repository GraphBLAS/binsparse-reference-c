#!/usr/bin/env sh

CONTAINER='binsparse-reference-c'
IMAGE='docker.io/junikimm717/nvim2025:finch'
DIR="$(realpath "$(dirname "$0")")"

case "$1" in
  pull|p)
    podman pull "$IMAGE"
    (podman container ls | grep "$CONTAINER" > /dev/null 2>&1) && podman container rm -fv "$CONTAINER"
    ;;
  clear|c)
    (podman container ls -a | grep "$CONTAINER" > /dev/null 2>&1)\
    && {
      podman container kill "$CONTAINER" > /dev/null 2>&1;
      podman container rm "$CONTAINER"
    }
    ;;
  *)
    set +x
    if ! (podman container ls -a | grep "$CONTAINER" > /dev/null 2>&1); then
      podman run\
        -dt\
        --name "$CONTAINER"\
        --group-add keep-groups\
        -v "$DIR:/workspace"\
        --privileged\
        --rm\
        "$IMAGE"
    fi || exit 1
    podman exec\
      -e ENV=/root/.profile\
      -it "$CONTAINER" /bin/bash
    ;;
esac
