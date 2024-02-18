#!/usr/bin/env bash

TT_DEFAULT_TERMINAL=${TT_DEFAULT_TERMINAL:-'gnome-terminal'}
if ! which ${TT_DEFAULT_TERMINAL} &> /dev/null; then
    echo "Error: Failed to open another terminal emulator!"
    echo "Info: Please set and export \$TT_DEFAULT_TERMINAL to your default terminal emulator"
    exit 1
fi

if ! which tmux &> /dev/null; then
    echo "Error: Failed to find tmux on your system!"
    echo "Info: Please install tmux for your system https://github.com/tmux/tmux/wiki"
    exit 1
fi

if [ -z "${TT_SKIP_DIMENSIONS_CHECK}" ]; then
    if ! which xdpyinfo &> /dev/null; then
        echo "Error: Failed to find xdpyinfo on your system!"
        echo "Info: Please install xdpyinfo manually or set \$TT_SKIP_DIMENSIONS_CHECK to true to skip screen resolution check"
        exit 1
    fi
    DIMENSIONS=$(xdpyinfo | grep dimensions | sed -r 's/^[^0-9]*([0-9]+x[0-9]+).*$/\1/')
    WIDTH=$(echo $DIMENSIONS | sed -r 's/x.*//')
    HEIGHT=$(echo $DIMENSIONS | sed -r 's/.*x//')
    if (( WIDTH < 1280 )) || (( HEIGHT < 720 )); then
        echo "Error: Your screen resolution is too low!"
        echo "Info: Your screen resolution is ${WIDTH}x${HEIGHT} but minimum required screen resolution is 1280x720"
        exit 1
    fi
fi

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
${TT_DEFAULT_TERMINAL} -- /usr/bin/env bash -c "${SCRIPTPATH}/tteams-main.sh"
exit 0