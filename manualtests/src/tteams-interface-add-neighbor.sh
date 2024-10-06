#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
INTERFACE_NAME="${1}"
MAC_ADDRESS="${2}"
IP_ADDRESS="${3}"

# Set neighbors
sudo ip neigh add ${IP_ADDRESS} lladdr ${MAC_ADDRESS} dev "${INTERFACE_NAME}"
sudo ip neigh show dev "${INTERFACE_NAME}"
