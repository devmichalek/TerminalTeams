#!/usr/bin/env bash

# Test setup
APP_HANDLER_STDIN=handler-stdin
APP_HANDLER_STDOUT=handler-stdout
SHARED_MEMORY_NAME=contacts
APP_HANDLER="./tteams-contacts-handler"
APP_HANDLER_ARGS=(0 0 "${SHARED_MEMORY_NAME}")
APP_HANDLER_CMD="${APP_HANDLER} ${APP_HANDLER_ARGS[@]}"
APP="./tteams-contacts"
APP_ARGS=(0 0 "${SHARED_MEMORY_NAME}")
APP_CMD="${APP} ${APP_ARGS[@]}"
mkfifo ${APP_HANDLER_STDIN}
sleep infinity > ${APP_HANDLER_STDIN} &
APP_HANDLER_STDIN_PID=$!
${APP_HANDLER_CMD} < "${APP_HANDLER_STDIN}" &
${APP_CMD} &> "${APP_HANDLER_STDOUT}" &

# Test scenario
echo "Info: Waiting for synchronization..."
sleep 5
echo "create John" > "${APP_HANDLER_STDIN}"
echo "select 1" > "${APP_HANDLER_STDIN}"
echo "create Camille" > "${APP_HANDLER_STDIN}"
echo "select 2" > "${APP_HANDLER_STDIN}"
echo "Info: Waiting for data to be set..."
sleep 3

# Expected output
EXPECTED_RESULTS_RAW="\033[2J\033[1;1H#0 John 
\033[2J\033[1;1H#0 John 
#1 Camille "
EXPECTED_RESULTS=$(echo -e "$EXPECTED_RESULTS_RAW")
ACTUAL_RESULTS=$(<"${APP_HANDLER_STDOUT}")

# Test teardown
kill $APP_HANDLER_STDIN_PID
echo "Info: Waiting for application to stop..."
sleep 2
echo "Info: Application shall be stopped now"
source tteams-contacts-autotests-verdict.sh