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

if ! which ip &> /dev/null; then
    echo "Error: Failed to find ip on your system!"
    echo "Info: Please install ip for your system"
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

usage()
{
    echo "Terminal Teams a chat between users on the LAN within Terminal Emulator." >&2
	echo "Usage ${0} [-i INTERFACE] [-a IP_ADDRESS]" >&2
	echo '-i  Interface that should be used for communication' >&2
	echo '-a  IP address that should be used within provided interface' >&2
	echo '-h  Displays this help' >&2
    echo "If no options are used script automatically detects neighbors on LAN using available interface." >&2
	exit 0
}

while getopts hi:a: OPTION
do
	case "${OPTION}" in
		i) INTERFACE=${OPTARG} ;;
		a) IP_ADDRESS=${OPTARG} ;;
        h) usage ;;
		?) echo 'Invalid option!' >&2; usage ;;
	esac
done

if ! [ -z "${IP_ADDRESS}" ]; then
    if [ -z "${INTERFACE}" ]; then
        echo "Error: Interface is not specified but IP address was provided!"
        echo "Info: Please specify interface you want to use using -i option"
        exit 1
    else
        IP_ADDRESSES=$(ip -f inet addr show ${INTERFACE} | sed -En -e 's/.*inet ([0-9.]+).*/\1/p')
        if [[ ! " ${IP_ADDRESSES[*]} " =~ [[:space:]]${IP_ADDRESS}[[:space:]] ]]; then
            echo "Error: Interface ${INTERFACE} doesn't have specified ${IP_ADDRESS} IP address"
            echo "Info: Please specify valid interface (-i option) and valid IP address (-a option)"
            exit 1
        fi
    fi
fi

if [ -z "${INTERFACE}" ]; then
    NEIGHBORS=$(ip neigh)
    if [ ${#NEIGHBORS[@]} -lt 1 ]; then
        echo "Error: No neighbors have been found!"
        echo "Info: Please specify interface you want to use using -i option"
        exit 1
    fi

    INTERFACES=$(awk -F ' ' '{print $3}' <<< "$NEIGHBORS")
    UNIQUE_INTERFACES=$(printf "$INTERFACES" | sort | uniq)
    if [ ${#UNIQUE_INTERFACES[@]} -gt 1 ]; then
        echo "Error: Multiple available interfaces found over multiple neighbors!"
        echo "Info: Please specify interface you want to use using -i option"
        exit 1
    fi
    INTERFACE=${UNIQUE_INTERFACES[0]}
fi

if [ -z "${IP_ADDRESS}" ]; then
    IP_ADDRESSES=$(ip -f inet addr show ${INTERFACE} | sed -En -e 's/.*inet ([0-9.]+).*/\1/p')
    if [ ${#IP_ADDRESSES[@]} -gt 1 ]; then
        echo "Error: Multiple available IP address found over $INTERFACE interface!"
        echo "Info: Please specify IP addrss you want to use using -a option"
        exit 1
    fi
    IP_ADDRESS=${IP_ADDRESSES[0]}
fi

echo "Using ${INTERFACE} interface and ${IP_ADDRESS} IP address..."
sleep 3

SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
${TT_DEFAULT_TERMINAL} -- /usr/bin/env bash -c "${SCRIPTPATH}/tteams-main.sh" "${INTERFACE}" "${IP_ADDRESS}"
exit 0