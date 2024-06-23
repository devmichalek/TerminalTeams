#!/usr/bin/env bash

# Test setup
HANDLER_STDIN=handler-stdin
HANDLER_STDOUT=handler-stdout
SHARED_MEMORY_NAME=contacts
APP_HANDLER="./tteams-contacts-handler"
APP_HANDLER_ARGS=(0 0 "${SHARED_MEMORY_NAME}")
APP_HANDLER_CMD="${APP_HANDLER} ${APP_HANDLER_ARGS[@]}"
APP="./tteams-contacts"
APP_ARGS=(0 0 "${SHARED_MEMORY_NAME}")
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

# Test teardown
kill $HANDLER_STDIN_PID
echo "Info: Waiting for application to stop..."
sleep 6
echo "Info: Application shall be stopped now"
source tteams-contacts-autotests-verdict.sh