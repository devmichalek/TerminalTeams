#!/usr/bin/env bash

# Test setup
APP_HANDLER_STDIN=handler-stdin
APP_HANDLER_STDOUT=handler-stdout
APP_STDOUT=app-stdout
UNIQUE_NAME=textbox
APP_HANDLER="./tteams-textbox-handler"
APP_HANDLER_ARGS=(0 0 "${UNIQUE_NAME}")
APP_HANDLER_CMD="${APP_HANDLER} ${APP_HANDLER_ARGS[@]}"
APP="./tteams-textbox"
APP_ARGS=(0 0 "${UNIQUE_NAME}")
APP_CMD="${APP} ${APP_ARGS[@]}"
mkfifo ${APP_HANDLER_STDIN}
sleep infinity > ${APP_HANDLER_STDIN} &
APP_HANDLER_STDIN_PID=$!
${APP_HANDLER_CMD} &> "${APP_HANDLER_STDOUT}" &
${APP_CMD} < "${APP_HANDLER_STDIN}" &> ${APP_STDOUT} &
EXIT_STATUS=0

# Test scenario
echo "Info: Waiting for synchronization..."
sleep 2
echo "#switch 0 blahblah" > "${APP_HANDLER_STDIN}"
echo "#switch blahblah" > "${APP_HANDLER_STDIN}"
echo "#switch 11111" > "${APP_HANDLER_STDIN}"
echo "Info: Waiting for data to be set..."
sleep 2

# Expected output
APP_HANDLER_EXPECTED_RESULTS_RAW=""
APP_HANDLER_EXPECTED_RESULTS=$(echo -e "$APP_HANDLER_EXPECTED_RESULTS_RAW")
APP_HANDLER_ACTUAL_RESULTS=$(<"${APP_HANDLER_STDOUT}")
APP_EXPECTED_RESULTS_RAW="\033[2J\033[1;1HType #help to print a help message
\033[2J\033[1;1H\033[2J\033[1;1H\033[2J\033[1;1H"
APP_EXPECTED_RESULTS=$(echo -e "$APP_EXPECTED_RESULTS_RAW")
APP_ACTUAL_RESULTS=$(<"${APP_STDOUT}")

# Test teardown
echo "#quit" > "${APP_HANDLER_STDIN}"
echo "Info: Waiting for application to stop..."
sleep 1
echo "Info: Application shall be stopped now"
source tteams-textbox-autotests-verdict.sh