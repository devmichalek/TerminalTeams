#!/usr/bin/env bash

# Test setup
APP_HANDLER_STDIN=app-handler-stdin
APP_HANDLER_STDOUT=app-handler-stdout
APP_STDOUT=app-stdout
MSG_QUEUE_NAME=chat
APP_HANDLER="./tteams-chat-handler"
APP_HANDLER_ARGS=(40 10 "${MSG_QUEUE_NAME}")
APP_HANDLER_CMD="${APP_HANDLER} ${APP_HANDLER_ARGS[@]}"
APP="./tteams-chat"
APP_ARGS=(40 10 "${MSG_QUEUE_NAME}")
APP_CMD="${APP} ${APP_ARGS[@]}"
mkfifo ${APP_HANDLER_STDIN}
sleep infinity > ${APP_HANDLER_STDIN} &
APP_HANDLER_STDIN_PID=$!
${APP_HANDLER_CMD} < "${APP_HANDLER_STDIN}" &> ${APP_HANDLER_STDOUT} &
${APP_CMD} &> "${APP_STDOUT}" &

# Test scenario
echo "Info: Waiting for synchronization..."
sleep 3
echo -e "create 0" > "${APP_HANDLER_STDIN}"
echo -e "create 1" > "${APP_HANDLER_STDIN}"
echo -e "select 0" > "${APP_HANDLER_STDIN}"
echo -e "send 0   \tHello John, how are you?    \t" > "${APP_HANDLER_STDIN}"
echo -e "receive 0 \t\t\tHi Freddie, good and you?\t   " > "${APP_HANDLER_STDIN}"
echo -e "send 0 Fine\t\t\t" > "${APP_HANDLER_STDIN}"
echo -e "select 1" > "${APP_HANDLER_STDIN}"
echo -e "send 1 What's up bro?        " > "${APP_HANDLER_STDIN}"
echo -e "receive 1 Nothing       " > "${APP_HANDLER_STDIN}"
echo -e "select 0" > "${APP_HANDLER_STDIN}"
echo "Info: Waiting for data to be set..."
sleep 3

# Expected output
APP_EXPECTED_RESULTS_RAW="                     1970-01-01 01:00:00
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
APP_EXPECTED_RESULTS=$(echo -e "$APP_EXPECTED_RESULTS_RAW")
APP_ACTUAL_RESULTS=$(<"${APP_STDOUT}")
APP_HANDLER_EXPECTED_RESULTS="create status=1
create status=1
select status=1
send status=1
receive status=1
send status=1
select status=1
send status=1
receive status=1
select status=1"
APP_HANDLER_ACTUAL_RESULTS=$(<"${APP_HANDLER_STDOUT}")

# Test teardown
kill $APP_HANDLER_STDIN_PID
echo "Info: Waiting for application to stop..."
sleep 3
echo "Info: Application shall be stopped now"
source tteams-chat-autotests-verdict.sh