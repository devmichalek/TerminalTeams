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
# -> ACTIVE, ACTIVE -> SELECTED_ACTIVE, -> ACTIVE, -> ACTIVE
echo "create John" > "${APP_HANDLER_STDIN}"
echo "select 0" > "${APP_HANDLER_STDIN}"
echo "create Camille" > "${APP_HANDLER_STDIN}"
echo "create Gabrielle" > "${APP_HANDLER_STDIN}"
# ACTIVE -> INACTIVE, SELECTED_ACTIVE -> ACTIVE, INACTIVE -> SELECTED_INACTIVE, ACTIVE -> UNREAD_MSG_ACTIVE, SELECTED_INACTIVE -> INACTIVE, UNREAD_MSG_ACTIVE -> SELECTED_ACTIVE
echo "deactivate 1" > "${APP_HANDLER_STDIN}"
echo "select 1" > "${APP_HANDLER_STDIN}"
echo "receive 2" > "${APP_HANDLER_STDIN}"
echo "select 2" > "${APP_HANDLER_STDIN}"
# ACTIVE -> UNREAD_MSG_ACTIVE, UNREAD_MSG_ACTIVE -> UNREAD_MSG_INACTIVE, SELECTED_ACTIVE -> ACTIVE, UNREAD_MSG_INACTIVE -> SELECTED_INACTIVE
echo "receive 0" > "${APP_HANDLER_STDIN}"
echo "deactivate 0" > "${APP_HANDLER_STDIN}"
echo "select 0" > "${APP_HANDLER_STDIN}"
# SELECTED_INACTIVE -> SELECTED_PENDING_MSG_INACTIVE, SELECTED_PENDING_MSG_INACTIVE -> PENDING_MSG_INACTIVE, PENDING_MSG_INACTIVE -> SELECTED_PENDING_MSG_INACTIVE
echo "send 0" > "${APP_HANDLER_STDIN}"
echo "select 0" > "${APP_HANDLER_STDIN}"
echo "Info: Waiting for data to be set..."
sleep 3

# Expected output
EXPECTED_RESULTS_RAW="\033[2J\033[1;1H#0 John 
\033[2J\033[1;1H#0 John <
\033[2J\033[1;1H#0 John <
#1 Camille 
\033[2J\033[1;1H#0 John <
#1 Camille 
#2 Gabrielle 
\033[2J\033[1;1H#0 John <
#1 Camille ?
#2 Gabrielle 
\033[2J\033[1;1H#0 John 
#1 Camille ?
#2 Gabrielle 
\033[2J\033[1;1H#0 John 
#1 Camille <?
#2 Gabrielle 
\033[2J\033[1;1H#0 John 
#1 Camille <?
#2 Gabrielle @
\033[2J\033[1;1H#0 John 
#1 Camille ?
#2 Gabrielle @
\033[2J\033[1;1H#0 John 
#1 Camille ?
#2 Gabrielle <
\033[2J\033[1;1H#0 John @
#1 Camille ?
#2 Gabrielle <
\033[2J\033[1;1H#0 John @?
#1 Camille ?
#2 Gabrielle <
\033[2J\033[1;1H#0 John @?
#1 Camille ?
#2 Gabrielle 
\033[2J\033[1;1H#0 John <?
#1 Camille ?
#2 Gabrielle 
\033[2J\033[1;1H#0 John <!?
#1 Camille ?
#2 Gabrielle 
\033[2J\033[1;1H#0 John !?
#1 Camille ?
#2 Gabrielle 
\033[2J\033[1;1H#0 John <!?
#1 Camille ?
#2 Gabrielle "
EXPECTED_RESULTS=$(echo -e "$EXPECTED_RESULTS_RAW")
ACTUAL_RESULTS=$(<"${APP_HANDLER_STDOUT}")

# Test teardown
kill $APP_HANDLER_STDIN_PID
echo "Info: Waiting for application to stop..."
sleep 2
echo "Info: Application shall be stopped now"
source tteams-contacts-autotests-verdict.sh