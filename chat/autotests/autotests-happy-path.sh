#!/usr/bin/env bash

# Test setup
APP_HANDLER="./tteams-chat-handler"
APP="./tteams-chat"
HANDLER_STDIN=handler-stdin
HANDLER_STDOUT=handler-stdout
MSG_QUEUE_NAME=chat
mkfifo ${HANDLER_STDIN}
sleep infinity > ${HANDLER_STDIN} &
HANDLER_STDIN_PID=$!
${APP_HANDLER} "${MSG_QUEUE_NAME}" < "${HANDLER_STDIN}" &
${APP} 40 10 "${MSG_QUEUE_NAME}" &> "${HANDLER_STDOUT}" &

# Test scenario
sleep 3 # Wait for synchronization
echo "create 0" > "${HANDLER_STDIN}"
echo "create 1" > "${HANDLER_STDIN}"
echo "clear 0" > "${HANDLER_STDIN}"
echo "send 0 Hello John, how are you?" > "${HANDLER_STDIN}"
echo "receive 0 Hi Freddie, good and you?" > "${HANDLER_STDIN}"
echo "send 0 Fine" > "${HANDLER_STDIN}"
echo "clear 1" > "${HANDLER_STDIN}"
echo "send 1 What's up bro?" > "${HANDLER_STDIN}"
echo "receive 1 Nothing" > "${HANDLER_STDIN}"
echo "clear 0" > "${HANDLER_STDIN}"
sleep 3 # Wait for data to be set

# Expected output
EXPECTED_RESULTS_RAW="\033[2J\033[1;1H                     1970-01-01 01:00:00
                Hello John, how are you?

1970-01-01 01:00:00
Hi Freddie, good and you?

                     1970-01-01 01:00:00
                                    Fine

\033[2J\033[1;1H                     1970-01-01 01:00:00
                          What's up bro?

1970-01-01 01:00:00
Nothing

\033[2J\033[1;1H                     1970-01-01 01:00:00
                Hello John, how are you?

1970-01-01 01:00:00
Hi Freddie, good and you?

                     1970-01-01 01:00:00
                                    Fine"
EXPECTED_RESULTS=$(echo -e "$EXPECTED_RESULTS_RAW")
ACTUAL_RESULTS=$(<"${HANDLER_STDOUT}")

# Test cleanup
kill $HANDLER_STDIN_PID
sleep 3 # Wait for stop

# Test verdict
EXIT_STATUS=0
APP_PID=$(pgrep -f ${APP} | head -n 1)
if [[ "${APP_PID}" ]]; then
    echo "Error: Application is still running! Killing pid=$APP_PID..."
    kill $APP_PID
    EXIT_STATUS=1
fi
APP_HANDLER_PID=$(pgrep -f ${APP_HANDLER} | head -n 1)
if [[ "${APP_HANDLER_PID}" ]]; then
    echo "Error: Application handler is still running! Killing pid=$APP_HANDLER_PID..."
    kill $APP_HANDLER_PID
    EXIT_STATUS=1
fi
MSG_QUEUE_PATH="/dev/mqueue/${MSG_QUEUE_NAME}"
if [ -f "${MSG_QUEUE_PATH}" ]; then
    echo "Error: File ${MSG_QUEUE_PATH} exists! Removing this file..."
    rm -f "${MSG_QUEUE_PATH}"
    EXIT_STATUS=1
fi
MSG_QUEUE_REVERSED_PATH="/dev/mqueue/${MSG_QUEUE_NAME}-reversed"
if [ -f "${MSG_QUEUE_REVERSED_PATH}" ]; then
    echo "Error: File ${MSG_QUEUE_REVERSED_PATH} exists! Removing this file..."
    rm -f "${MSG_QUEUE_PATH}"
    EXIT_STATUS=1
fi
if [[ "$ACTUAL_RESULTS" != "$EXPECTED_RESULTS" ]]; then
    echo "Error: Test failed!"
    printf "%s" "$ACTUAL_RESULTS" > actual_results.txt
    printf "%s" "$EXPECTED_RESULTS" > expected_results.txt
    EXIT_STATUS=1
fi
if [[ $EXIT_STATUS -eq 0 ]]; then
    echo "Success: Test passed!"
    rm -f "${HANDLER_STDIN}"
    rm -f "${HANDLER_STDOUT}"
fi
exit $EXIT_STATUS