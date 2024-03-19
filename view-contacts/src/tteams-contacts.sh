#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
SHARED_MEMORY_NAME="${1:-contacts}" # todo: remove default value
${SCRIPTPATH}/tteams-contacts $(tput cols) $(tput lines) "${SHARED_MEMORY_NAME}"