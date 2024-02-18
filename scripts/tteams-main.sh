#!/usr/bin/env bash

tmux start-server

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
TMUX_SESSION_NAME="terminal-teams-session"

tmux kill-session -t ${TMUX_SESSION_NAME}
tmux new-session -d -x "$(tput cols)" -y "$(tput lines)" -s ${TMUX_SESSION_NAME} "/usr/bin/env bash -c ${SCRIPTPATH}/tteams-contacts.sh"

tmux split-window -p 75 -h -t ${TMUX_SESSION_NAME}:0 "/usr/bin/env bash -c ${SCRIPTPATH}/tteams-chat.sh"
tmux split-window -p 20 -t ${TMUX_SESSION_NAME}:0 "/usr/bin/env bash -c ${SCRIPTPATH}/tteams-textbox.sh"

tmux attach -t${TMUX_SESSION_NAME}
sleep 3