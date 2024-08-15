#!/usr/bin/env bash

# Test setup
HANDLER_STDIN=handler-stdin
HANDLER_STDOUT=handler-stdout
MSG_QUEUE_NAME=chat
APP_HANDLER="./tteams-chat-handler"
APP_HANDLER_ARGS=(40 10 "${MSG_QUEUE_NAME}")
APP_HANDLER_CMD="${APP_HANDLER} ${APP_HANDLER_ARGS[@]}"
APP="./tteams-chat"
APP_ARGS=(40 10 "${MSG_QUEUE_NAME}")
APP_CMD="${APP} ${APP_ARGS[@]}"
mkfifo ${HANDLER_STDIN}
sleep infinity > ${HANDLER_STDIN} &
HANDLER_STDIN_PID=$!
${APP_HANDLER_CMD} < "${HANDLER_STDIN}" &
${APP_CMD} &> "${HANDLER_STDOUT}" &

# Test scenario
echo "Info: Waiting for synchronization..."
sleep 3
echo -e "create 0" > "${HANDLER_STDIN}"
echo -e "create 1" > "${HANDLER_STDIN}"
echo -e "select 0" > "${HANDLER_STDIN}"
echo -e "send 0   \tHello John, how are you?    \t" > "${HANDLER_STDIN}"
echo -e "receive 0 \t\t\tHi Freddie, good and you?\t   " > "${HANDLER_STDIN}"
echo -e "send 0 Fine\t\t\t" > "${HANDLER_STDIN}"
echo -e "select 1" > "${HANDLER_STDIN}"
echo -e "send 1 What's up bro?        " > "${HANDLER_STDIN}"
echo -e "receive 1 Nothing       " > "${HANDLER_STDIN}"
echo -e "select 0" > "${HANDLER_STDIN}"
echo "Info: Waiting for data to be set..."
sleep 3

# Expected output
EXPECTED_RESULTS_RAW="                     1970-01-01 01:00:00
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

# Test teardown
kill $HANDLER_STDIN_PID
echo "Info: Waiting for application to stop..."
sleep 3
echo "Info: Application shall be stopped now"
source tteams-chat-autotests-verdict.sh