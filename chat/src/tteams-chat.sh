#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
MSG_QUEUE_NAME="${1}"
${SCRIPTPATH}/tteams-chat $(tput cols) $(tput lines) "/${MSG_QUEUE_NAME}"

EXIT_STATUS=0
MSG_QUEUE_PATH="/dev/mqueue/${MSG_QUEUE_NAME}"
if [ -f "${MSG_QUEUE_PATH}" ]; then
    rm -f "${MSG_QUEUE_PATH}"
    EXIT_STATUS=1
fi
MSG_QUEUE_REVERSED_PATH="/dev/mqueue/${MSG_QUEUE_NAME}-reversed"
if [ -f "${MSG_QUEUE_REVERSED_PATH}" ]; then
    rm -f "${MSG_QUEUE_PATH}"
    EXIT_STATUS=1
fi
exit $EXIT_STATUS
