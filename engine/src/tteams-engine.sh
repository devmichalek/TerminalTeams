#!/usr/bin/env bash

tmux start-server

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
INTERFACE="${1}"
IP_ADDRESS="${2}"
PORT="${3}"
NICKNAME="${4}"
IDENTITY="${5}"
TMUX_SESSION_NAME="terminal-teams-session"
CONTACTS_SHARED_MEMORY_NAME="$(hostname)-contacts" # todo add mechanism for generating random shared name
CHAT_MESSAGE_QUEUE_NAME="$(hostname)-chat" # todo add mechanism for generating random queue name
TEXTBOX_PIPE_NAME="$(hostname)-textbox" # todo add mechanism for generating random pipe name
NEIGHBORS_RAW=$(ip neigh | grep ${INTERFACE})
NEIGHBORS=$(awk -F ' ' '{print $1}' <<< "$NEIGHBORS_RAW")

${SCRIPTPATH}/tteams-engine ${CONTACTS_SHARED_MEMORY_NAME} ${CHAT_MESSAGE_QUEUE_NAME} ${TEXTBOX_PIPE_NAME} "${NICKNAME}" "${IDENTITY}" ${INTERFACE} ${IP_ADDRESS} ${PORT} ${NEIGHBORS[@]} &>/dev/null &

tmux kill-session -t ${TMUX_SESSION_NAME}
tmux new-session -d -x "$(tput cols)" -y "$(tput lines)" -s ${TMUX_SESSION_NAME} "/usr/bin/env bash -c ${SCRIPTPATH}/tteams-contacts.sh ${CONTACTS_SHARED_MEMORY_NAME}
tmux split-window -p 75 -h -t ${TMUX_SESSION_NAME}:0 "/usr/bin/env bash -c ${SCRIPTPATH}/tteams-chat.sh ${CHAT_MESSAGE_QUEUE_NAME}"
tmux split-window -p 20 -t ${TMUX_SESSION_NAME}:0 "/usr/bin/env bash -c ${SCRIPTPATH}/tteams-textbox.sh ${TEXTBOX_PIPE_NAME}"
tmux attach -t${TMUX_SESSION_NAME}

sleep 3