#!/usr/bin/env bash

EXIT_STATUS=0
APP_PID=$(pgrep -f "${APP_CMD}" | head -n 1)
if [[ "${APP_PID}" ]]; then
    echo "Error: Application is still running! Killing pid=$APP_PID..."
    kill -9 $APP_PID
    EXIT_STATUS=1
fi
APP_HANDLER_PID=$(pgrep -f "${APP_HANDLER_CMD}" | head -n 1)
if [[ "${APP_HANDLER_PID}" ]]; then
    echo "Error: Application handler is still running! Killing pid=$APP_HANDLER_PID..."
    kill -9 $APP_HANDLER_PID
    EXIT_STATUS=1
fi
SHARED_MEMORY_PATH="/dev/shm/${SHARED_MEMORY_NAME}"
if [ -f "${SHARED_MEMORY_PATH}" ]; then
    echo "Error: File ${SHARED_MEMORY_PATH} exists! Removing this file..."
    rm -f "${SHARED_MEMORY_PATH}"
    EXIT_STATUS=1
fi
SEM_DATA_CONSUMED_PATH="/dev/shm/sem.${SHARED_MEMORY_NAME}-data-consumed"
if [ -f "${SEM_DATA_CONSUMED_PATH}" ]; then
    echo "Error: File ${SEM_DATA_CONSUMED_PATH} exists! Removing this file..."
    rm -f "${SEM_DATA_CONSUMED_PATH}"
    EXIT_STATUS=1
fi
SEM_DATA_PRODUCED_PATH="/dev/shm/sem.${SHARED_MEMORY_NAME}-data-produced"
if [ -f "${SEM_DATA_PRODUCED_PATH}" ]; then
    echo "Error: File ${SEM_DATA_PRODUCED_PATH} exists! Removing this file..."
    rm -f "${SEM_DATA_PRODUCED_PATH}"
    EXIT_STATUS=1
fi
if [[ "$ACTUAL_RESULTS" != "$EXPECTED_RESULTS" ]]; then
    echo "Error: Actual results are different than expected!"
    printf "%s" "$ACTUAL_RESULTS" > actual_results.txt
    printf "%s" "$EXPECTED_RESULTS" > expected_results.txt
    EXIT_STATUS=1
fi

if [[ $EXIT_STATUS -eq 0 ]]; then
    echo "Success: Autotest passed!"
    rm -f "${HANDLER_STDIN}"
    rm -f "${HANDLER_STDOUT}"
fi
exit $EXIT_STATUS