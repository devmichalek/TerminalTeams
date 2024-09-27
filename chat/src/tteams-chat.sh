#!/usr/bin/env bash

if [[ $# -ne 1 ]]; then
    echo "Error: Illegal number of parameters!" >&2
    exit 2
fi

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
MSG_QUEUE_NAME="${1}"
MSG_QUEUE_PRIMARY_PATH="/dev/mqueue/${MSG_QUEUE_NAME}-primary"
MSG_QUEUE_SECONDARY_PATH="/dev/mqueue/${MSG_QUEUE_NAME}-secondary"

${SCRIPTPATH}/tteams-chat $(tput cols) $(tput lines) "/${MSG_QUEUE_NAME}"

EXIT_STATUS=0
if [ -f "${MSG_QUEUE_PRIMARY_PATH}" ]; then
    rm -f "${MSG_QUEUE_PRIMARY_PATH}"
    EXIT_STATUS=1
fi
if [ -f "${MSG_QUEUE_SECONDARY_PATH}" ]; then
    rm -f "${MSG_QUEUE_SECONDARY_PATH}"
    EXIT_STATUS=1
fi
exit $EXIT_STATUS
