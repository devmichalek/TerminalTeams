#!/usr/bin/env bash

# todo: check /dev/mqueue

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
MESSAGE_QUEUE_NAME="${1:-chat}" # todo: remove default value
${SCRIPTPATH}/tteams-chat $(tput cols) $(tput lines) "/${MESSAGE_QUEUE_NAME}"