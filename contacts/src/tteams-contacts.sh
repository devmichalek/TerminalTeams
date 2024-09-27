#!/usr/bin/env bash

if [[ $# -ne 1 ]]; then
    echo "Error: Illegal number of parameters!" >&2
    exit 2
fi

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
SHARED_MEMORY_NAME="${1}"
SHARED_MEMORY_PATH="/dev/shm/${SHARED_MEMORY_NAME}"
SEM_DATA_CONSUMED_PATH="/dev/shm/sem.${SHARED_MEMORY_NAME}-data-consumed"
SEM_DATA_PRODUCED_PATH="/dev/shm/sem.${SHARED_MEMORY_NAME}-data-produced"

${SCRIPTPATH}/tteams-contacts $(tput cols) $(tput lines) "${SHARED_MEMORY_NAME}"

EXIT_STATUS=0
if [ -f "${SHARED_MEMORY_PATH}" ]; then
    rm -f "${SHARED_MEMORY_PATH}"
    EXIT_STATUS=1
fi
if [ -f "${SEM_DATA_CONSUMED_PATH}" ]; then
    rm -f "${SEM_DATA_CONSUMED_PATH}"
    EXIT_STATUS=1
fi
if [ -f "${SEM_DATA_PRODUCED_PATH}" ]; then
    rm -f "${SEM_DATA_PRODUCED_PATH}"
    EXIT_STATUS=1
fi
exit $EXIT_STATUS