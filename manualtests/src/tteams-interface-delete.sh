#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
INTERFACE_NAME="${1}"

sudo ip link del dev "${INTERFACE_NAME}"
sudo ip link show dev "${INTERFACE_NAME}" 2> /dev/null
