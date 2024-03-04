#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
QUEUE_NAME="${1:-chat}" # todo: remove default value
${SCRIPTPATH}/tteams-chat $(tput cols) $(tput lines) "${QUEUE_NAME}"