#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
INTERFACE_NAME="${1}"
MAC_ADDRESS="${2}"
IP_ADDRESS="${3}"
IP_MASK="${4}"

# Enable dummy kernel module
sudo modprobe dummy

# Add interfaces
sudo ip link add "${INTERFACE_NAME}" type dummy
sudo ip link show "${INTERFACE_NAME}"

# Set MAC addresses
sudo ip link set dev ${INTERFACE_NAME} down
sudo ip link set dev ${INTERFACE_NAME} address "${MAC_ADDRESS}"
sudo ip link set dev ${INTERFACE_NAME} up
sudo ip link show dev "${INTERFACE_NAME}"

# Set IP addresses and labels
sudo ip addr add ${IP_ADDRESS}/${IP_MASK} dev ${INTERFACE_NAME}
sudo ip addr show dev "${INTERFACE_NAME}"
