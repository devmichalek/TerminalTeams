#!/usr/bin/env bash

# Test setup
HANDLER_STDIN=handler-stdin
HANDLER_STDOUT=handler-stdout
MSG_QUEUE_NAME=chat
mkfifo ${HANDLER_STDIN}
sleep infinity > ${HANDLER_STDIN} &
HANDLER_STDIN_PID=$!
./tteams-chat-handler "${MSG_QUEUE_NAME}" < "${HANDLER_STDIN}" &
./tteams-chat 40 10 "${MSG_QUEUE_NAME}" &> "${HANDLER_STDOUT}" &

# Test scenario
sleep 3 # Wait for synchronization
echo "create 0" > "${HANDLER_STDIN}"
echo "create 1" > "${HANDLER_STDIN}"
echo "clear 0" > "${HANDLER_STDIN}"
echo "send 0 Hello John, how are you?" > "${HANDLER_STDIN}"
echo "receive 0 Hi Freddie, good and you?" > "${HANDLER_STDIN}"
echo "send 0 Fine" > "${HANDLER_STDIN}"
echo "clear 1" > "${HANDLER_STDIN}"
echo "send 1 What's up bro?" > "${HANDLER_STDIN}"
echo "receive 1 Nothing" > "${HANDLER_STDIN}"
echo "clear 0" > "${HANDLER_STDIN}"
sleep 3 # Wait for data to be set

# Expected output
EXPECTED_RESULTS_RAW="\033[2J\033[1;1H                     1970-01-01 01:00:00
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

# Test cleanup
kill $HANDLER_STDIN_PID
wait $(pidof ./tteams-chat-handler)
wait $(pidof ./tteams-chat)
rm -f ${HANDLER_STDIN}
rm -f ${HANDLER_STDOUT}

# Test verdict
if [ -f "/dev/mqueue/${MSG_QUEUE_NAME}" ]; then
    echo "Error: File /dev/mqueue/${MSG_QUEUE_NAME} exists!"
    exit 1
fi
if [ -f "/dev/mqueue/${MSG_QUEUE_NAME}-reversed" ]; then
    echo "Error: File /dev/mqueue/${MSG_QUEUE_NAME}-reversed exists!"
    exit 1
fi
if [[ "$ACTUAL_RESULTS" == "$EXPECTED_RESULTS" ]]; then
    echo "Success: Test passed!"
    exit 0
else
    echo "Error: Test failed!"
    printf "%s" "$ACTUAL_RESULTS" > actual_results.txt
    printf "%s" "$EXPECTED_RESULTS" > expected_results.txt
    exit 1
fi

