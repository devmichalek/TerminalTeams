#!/usr/bin/env bash

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
${SCRIPTPATH}/tteams-chat $(tput cols) $(tput lines)