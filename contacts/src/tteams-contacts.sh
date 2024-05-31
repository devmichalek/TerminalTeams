#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
SHARED_MEMORY_NAME="${1}"
${SCRIPTPATH}/tteams-contacts $(tput cols) $(tput lines) "${SHARED_MEMORY_NAME}" "placeholder" "placeholder" "placeholder" "0"

EXIT_STATUS=0
SHARED_MEMORY_PATH="/dev/shm/${SHARED_MEMORY_NAME}"
if [ -f "${SHARED_MEMORY_PATH}" ]; then
    rm -f "${SHARED_MEMORY_PATH}"
    EXIT_STATUS=1
fi
SEM_DATA_CONSUMED_PATH="/dev/shm/sem.${SHARED_MEMORY_NAME}-data-consumed"
if [ -f "${SEM_DATA_CONSUMED_PATH}" ]; then
    rm -f "${SEM_DATA_CONSUMED_PATH}"
    EXIT_STATUS=1
fi
SEM_DATA_PRODUCED_PATH="/dev/shm/sem.${SHARED_MEMORY_NAME}-data-produced"
if [ -f "${SEM_DATA_PRODUCED_PATH}" ]; then
    rm -f "${SEM_DATA_PRODUCED_PATH}"
    EXIT_STATUS=1
fi
exit $EXIT_STATUS