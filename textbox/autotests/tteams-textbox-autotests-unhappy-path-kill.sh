#!/usr/bin/env bash

# Test setup
APP_HANDLER_STDIN=handler-stdin
APP_HANDLER_STDOUT=handler-stdout
UNIQUE_NAME=textbox
APP_HANDLER="./tteams-textbox-handler"
APP_HANDLER_ARGS=(0 0 "${UNIQUE_NAME}")
APP_HANDLER_CMD="${APP_HANDLER} ${APP_HANDLER_ARGS[@]}"
APP="./tteams-textbox"
APP_ARGS=(0 0 "${UNIQUE_NAME}")
APP_CMD="${APP} ${APP_ARGS[@]}"
${APP_HANDLER_CMD} &> "${APP_HANDLER_STDOUT}" &
${APP_CMD} &> /dev/null &
EXIT_STATUS=0

# Test scenario
echo "Info: Waiting for synchronization..."
sleep 3

# Expected output
APP_HANDLER_EXPECTED_RESULTS_RAW=""
APP_HANDLER_EXPECTED_RESULTS=$(echo -e "$APP_HANDLER_EXPECTED_RESULTS_RAW")
APP_HANDLER_ACTUAL_RESULTS=$(<"${APP_HANDLER_STDOUT}")
APP_EXPECTED_RESULTS=""
APP_ACTUAL_RESULTS=""

# Test teardown
kill "$(pgrep -f "${APP_HANDLER_CMD}" | head -n 1)"
echo "Info: Waiting for application to stop..."
sleep 1
echo "Info: Application shall be stopped now"
source tteams-textbox-autotests-verdict.sh