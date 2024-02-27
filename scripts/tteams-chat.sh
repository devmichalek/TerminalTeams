#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
INTERFACE="$1"
IP_ADDRESS="$2"
${SCRIPTPATH}/tteams-chat $(tput cols) $(tput lines) "${INTERFACE}" "${IP_ADDRESS}"