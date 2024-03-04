#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
PIPE_NAME="${1:-textbox}" # todo: remove default value
${SCRIPTPATH}/tteams-textbox $(tput cols) $(tput lines) "${PIPE_NAME}"