#!/usr/bin/env bash

# Test setup
HANDLER_STDIN=handler-stdin
HANDLER_STDOUT=handler-stdout
APP_STDOUT=app-stdout
UNIQUE_NAME=textbox
APP_HANDLER="./tteams-textbox-handler"
APP_HANDLER_ARGS=("${UNIQUE_NAME}")
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
sleep 3 # Wait for synchronization
echo "#0" > "${HANDLER_STDIN}"
echo "Hello John, how are you?" > "${HANDLER_STDIN}"
echo "Hi Freddie, good and you?" > "${HANDLER_STDIN}"
echo "Fine" > "${HANDLER_STDIN}"
echo "#1" > "${HANDLER_STDIN}"
echo "What's up bro?" > "${HANDLER_STDIN}"
echo "Nevermind" > "${HANDLER_STDIN}"
sleep 3 # Wait for data to be set

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

# Test cleanup
kill $HANDLER_STDIN_PID
sleep 3 # Wait for stop

# Test verdict
EXIT_STATUS=0
APP_PID=$(pgrep -f "${APP_CMD}" | head -n 1)
if [[ "${APP_PID}" ]]; then
    echo "Error: Application is still running! Killing pid=$APP_PID..."
    kill $APP_PID
    EXIT_STATUS=1
fi
APP_HANDLER_PID=$(pgrep -f "${APP_HANDLER_CMD}" | head -n 1)
if [[ "${APP_HANDLER_PID}" ]]; then
    echo "Error: Application handler is still running! Killing pid=$APP_HANDLER_PID..."
    kill $APP_HANDLER_PID
    EXIT_STATUS=1
fi
UNIQUE_PATH="/tmp/${UNIQUE_NAME}-pipe"
if [ -f "${UNIQUE_PATH}" ]; then
    echo "Error: File ${UNIQUE_PATH} exists! Removing this file..."
    rm -f "${UNIQUE_PATH}"
    EXIT_STATUS=1
fi
if [[ "$ACTUAL_RESULTS" != "$EXPECTED_RESULTS" ]]; then
    echo "Error: Test failed!"
    printf "%s" "$ACTUAL_RESULTS" > actual_results.txt
    printf "%s" "$EXPECTED_RESULTS" > expected_results.txt
    EXIT_STATUS=1
fi
if [[ $EXIT_STATUS -eq 0 ]]; then
    echo "Success: Autotest \"happy path\" passed!"
    rm -f "${HANDLER_STDIN}"
    rm -f "${HANDLER_STDOUT}"
    rm -f "${APP_STDOUT}"
fi
exit $EXIT_STATUS