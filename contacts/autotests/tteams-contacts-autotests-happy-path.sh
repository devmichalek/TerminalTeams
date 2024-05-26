#!/usr/bin/env bash

# Test setup
HANDLER_STDIN=handler-stdin
HANDLER_STDOUT=handler-stdout
SHARED_NAME=contacts
APP_HANDLER="./tteams-contacts-handler"
APP_HANDLER_ARGS=(0 0 "${SHARED_NAME}")
APP_HANDLER_CMD="${APP_HANDLER} ${APP_HANDLER_ARGS[@]}"
APP="./tteams-contacts"
APP_ARGS=(0 0 "${SHARED_NAME}")
APP_CMD="${APP} ${APP_ARGS[@]}"
mkfifo ${HANDLER_STDIN}
sleep infinity > ${HANDLER_STDIN} &
HANDLER_STDIN_PID=$!
${APP_HANDLER_CMD} < "${HANDLER_STDIN}" &
${APP_CMD} &> "${HANDLER_STDOUT}" &

# Test scenario
echo "Info: Waiting for synchronization..."
sleep 5
echo "create John" > "${HANDLER_STDIN}"
echo "create Camille" > "${HANDLER_STDIN}"
echo "Info: Waiting for data to be set..."
sleep 3

# Expected output
EXPECTED_RESULTS_RAW="\033[2J\033[1;1H#0 John 
\033[2J\033[1;1H#0 John 
#1 Camille "
EXPECTED_RESULTS=$(echo -e "$EXPECTED_RESULTS_RAW")
ACTUAL_RESULTS=$(<"${HANDLER_STDOUT}")

# Test cleanup
kill $HANDLER_STDIN_PID
echo "Info: Waiting for application to stop..."
sleep 6
echo "Info: Application shall be stopped now"

# Test verdict
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
SHARED_PATH="/dev/shm/${SHARED_NAME}"
if [ -f "${SHARED_PATH}" ]; then
    echo "Error: File ${SHARED_PATH} exists! Removing this file..."
    rm -f "${SHARED_PATH}"
    EXIT_STATUS=1
fi
SEM_DATA_CONSUMED_PATH="/dev/shm/sem.${SHARED_NAME}-data-consumed"
if [ -f "${SEM_DATA_CONSUMED_PATH}" ]; then
    echo "Error: File ${SEM_DATA_CONSUMED_PATH} exists! Removing this file..."
    rm -f "${SEM_DATA_CONSUMED_PATH}"
    EXIT_STATUS=1
fi
SEM_DATA_PRODUCED_PATH="/dev/shm/sem.${SHARED_NAME}-data-produced"
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
    echo "Success: Autotest \"happy path\" passed!"
    rm -f "${HANDLER_STDIN}"
    rm -f "${HANDLER_STDOUT}"
fi
exit $EXIT_STATUS