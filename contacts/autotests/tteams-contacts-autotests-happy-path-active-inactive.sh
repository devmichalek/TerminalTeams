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
# -> ACTIVE, ACTIVE -> SELECTED_ACTIVE, -> ACTIVE, -> ACTIVE
echo "create John" > "${HANDLER_STDIN}"
echo "select 0" > "${HANDLER_STDIN}"
echo "create Camille" > "${HANDLER_STDIN}"
echo "create Gabrielle" > "${HANDLER_STDIN}"
# ACTIVE -> OK, ACTIVE -> INACTIVE, INACTIVE -> OK, INACTIVE -> ACTIVE
echo "activate 1" > "${HANDLER_STDIN}"
echo "deactivate 1" > "${HANDLER_STDIN}"
echo "deactivate 1" > "${HANDLER_STDIN}"
echo "activate 1" > "${HANDLER_STDIN}"
# SELECTED_ACTIVE -> OK, SELECTED_ACTIVE -> SELECTED_INACTIVE, SELECTED_INACTIVE -> OK, SELECTED_INACTIVE -> SELECTED_ACTIVE
echo "activate 0" > "${HANDLER_STDIN}"
echo "deactivate 0" > "${HANDLER_STDIN}"
echo "deactivate 0" > "${HANDLER_STDIN}"
echo "activate 0" > "${HANDLER_STDIN}"
# ACTIVE -> UNREAD_MSG_ACTIVE, UNREAD_MSG_ACTIVE -> OK, UNREAD_MSG_ACTIVE -> UNREAD_MSG_INACTIVE, UNREAD_MSG_INACTIVE -> OK, UNREAD_MSG_INACTIVE -> UNREAD_MSG_ACTIVE
echo "receive 2" > "${HANDLER_STDIN}"
echo "activate 2" > "${HANDLER_STDIN}"
echo "deactivate 2" > "${HANDLER_STDIN}"
echo "deactivate 2" > "${HANDLER_STDIN}"
echo "activate 2" > "${HANDLER_STDIN}"
# SELECTED_ACTIVE -> SELECTED_INACTIVE, SELECTED_INACTIVE -> SELECTED_PENDING_MSG_INACTIVE, SELECTED_PENDING_MSG_INACTIVE -> PENDING_MSG_INACTIVE, UNREAD_MSG_ACTIVE -> SELECTED_ACTIVE
echo "deactivate 0" > "${HANDLER_STDIN}"
echo "send 0" > "${HANDLER_STDIN}"
echo "select 2" > "${HANDLER_STDIN}"
# PENDING_MSG_INACTIVE -> OK, PENDING_MSG_INACTIVE -> ACTIVE, SELECTED_ACTIVE -> SELECTED_INACTIVE, SELECTED_INACTIVE -> SELECTED_PENDING_MSG_INACTIVE
echo "deactivate 0" > "${HANDLER_STDIN}"
echo "activate 0" > "${HANDLER_STDIN}"
echo "deactivate 2" > "${HANDLER_STDIN}"
echo "send 2" > "${HANDLER_STDIN}"
# SELECTED_PENDING_MSG_INACTIVE -> OK, SELECTED_PENDING_MSG_INACTIVE -> SELECTED_ACTIVE
echo "deactivate 2" > "${HANDLER_STDIN}"
echo "activate 2" > "${HANDLER_STDIN}"
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
\033[2J\033[1;1H#0 John <
#1 Camille 
#2 Gabrielle 
\033[2J\033[1;1H#0 John <?
#1 Camille 
#2 Gabrielle 
\033[2J\033[1;1H#0 John <
#1 Camille 
#2 Gabrielle 
\033[2J\033[1;1H#0 John <
#1 Camille 
#2 Gabrielle @
\033[2J\033[1;1H#0 John <
#1 Camille 
#2 Gabrielle @?
\033[2J\033[1;1H#0 John <
#1 Camille 
#2 Gabrielle @
\033[2J\033[1;1H#0 John <?
#1 Camille 
#2 Gabrielle @
\033[2J\033[1;1H#0 John <!?
#1 Camille 
#2 Gabrielle @
\033[2J\033[1;1H#0 John !?
#1 Camille 
#2 Gabrielle @
\033[2J\033[1;1H#0 John !?
#1 Camille 
#2 Gabrielle <
\033[2J\033[1;1H#0 John 
#1 Camille 
#2 Gabrielle <
\033[2J\033[1;1H#0 John 
#1 Camille 
#2 Gabrielle <?
\033[2J\033[1;1H#0 John 
#1 Camille 
#2 Gabrielle <!?
\033[2J\033[1;1H#0 John 
#1 Camille 
#2 Gabrielle <"
EXPECTED_RESULTS=$(echo -e "$EXPECTED_RESULTS_RAW")
ACTUAL_RESULTS=$(<"${HANDLER_STDOUT}")

# Test teardown
kill $HANDLER_STDIN_PID
echo "Info: Waiting for application to stop..."
sleep 2
echo "Info: Application shall be stopped now"
source tteams-contacts-autotests-verdict.sh