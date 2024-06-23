#!/usr/bin/env bash

# Test setup
HANDLER_STDIN=handler-stdin
HANDLER_STDOUT=handler-stdout
APP_STDOUT=app-stdout
UNIQUE_NAME=textbox
APP_HANDLER="./tteams-textbox-handler"
APP_HANDLER_ARGS=(0 0 "${UNIQUE_NAME}")
APP_HANDLER_CMD="${APP_HANDLER} ${APP_HANDLER_ARGS[@]}"
APP="./tteams-textbox"
APP_ARGS=(0 0 "${UNIQUE_NAME}")
APP_CMD="${APP} ${APP_ARGS[@]}"
mkfifo ${HANDLER_STDIN}
sleep infinity > ${HANDLER_STDIN} &
HANDLER_STDIN_PID=$!
${APP_HANDLER_CMD} &> "${HANDLER_STDOUT}" &
${APP_CMD} < "${HANDLER_STDIN}" &> ${APP_STDOUT} &

# Test scenario
echo "Info: Waiting for synchronization..."
sleep 3
echo "#switch 0" > "${HANDLER_STDIN}"
echo "Hello John, how are you?" > "${HANDLER_STDIN}"
echo "Hi Freddie, good and you?" > "${HANDLER_STDIN}"
echo "Fine" > "${HANDLER_STDIN}"
echo "#switch 1" > "${HANDLER_STDIN}"
echo "What's up bro?" > "${HANDLER_STDIN}"
echo "Nevermind" > "${HANDLER_STDIN}"
echo "Info: Waiting for data to be set..."
sleep 3

# Expected output
EXPECTED_RESULTS_RAW="#0
Hello John, how are you?
Hi Freddie, good and you?
Fine
#1
What's up bro?
Nevermind"
EXPECTED_RESULTS=$(echo -e "$EXPECTED_RESULTS_RAW")
ACTUAL_RESULTS=$(<"${HANDLER_STDOUT}")

# Test teardown
echo "#quit" > "${HANDLER_STDIN}"
kill $HANDLER_STDIN_PID
echo "Info: Waiting for application to stop..."
sleep 6
echo "Info: Application shall be stopped now"
source tteams-textbox-autotests-verdict.sh