#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
INTERFACE="${1:-eno1}" # todo: remove default values
IP_ADDRESS="${2:-192.168.0.119}"
PORT="${3:-17888}"

${SCRIPTPATH}/tteams-chat $(tput cols) $(tput lines) "${INTERFACE}" "${IP_ADDRESS}" "${PORT}"