#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
UNIQUE_NAME="${1}"
${SCRIPTPATH}/tteams-textbox $(tput cols) $(tput lines) "${UNIQUE_NAME}"

EXIT_STATUS=0
UNIQUE_PATH="/tmp/${UNIQUE_NAME}-pipe"
if [ -f "${UNIQUE_PATH}" ]; then
    rm -f "${UNIQUE_PATH}"
    EXIT_STATUS=1
fi
exit $EXIT_STATUS
