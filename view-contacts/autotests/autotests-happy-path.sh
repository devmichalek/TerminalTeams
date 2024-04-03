#!/usr/bin/env bash

# Test setup
HANDLER_STDIN=handler-stdin
HANDLER_STDOUT=handler-stdout
SHARED_NAME=contacts
mkfifo ${HANDLER_STDIN}
sleep infinity > ${HANDLER_STDIN} &
HANDLER_STDIN_PID=$!
./tteams-contacts-handler "${SHARED_NAME}" < "${HANDLER_STDIN}" &
./tteams-contacts 0 0 "${SHARED_NAME}" &> "${HANDLER_STDOUT}" &

# Test scenario
sleep 3 # Wait for synchronization
echo "create John" > "${HANDLER_STDIN}"
echo "create Camille" > "${HANDLER_STDIN}"
sleep 3 # Wait for data to be set

# Expected output
EXPECTED_RESULTS_RAW="\033[2J\033[1;1H#0 John 
\033[2J\033[1;1H#0 John 
#1 Camille "
EXPECTED_RESULTS=$(echo -e "$EXPECTED_RESULTS_RAW")
ACTUAL_RESULTS=$(<"${HANDLER_STDOUT}")

# Test cleanup
kill $HANDLER_STDIN_PID
wait $(pidof ./tteams-contacts-handler)
wait $(pidof ./tteams-contacts)
rm -f ${HANDLER_STDIN}
rm -f ${HANDLER_STDOUT}

# Test verdict
if [ -f "/dev/shm/${SHARED_NAME}" ]; then
    echo "Error: File /dev/shm/${SHARED_NAME} exists!"
    exit 1
fi
if [ -f "/dev/shm/sem.${SHARED_NAME}-data-consumed" ]; then
    echo "Error: File /dev/shm/sem.${SHARED_NAME}-data-consumed exists!"
    exit 1
fi
if [ -f "/dev/shm/sem.${SHARED_NAME}-data-produced" ]; then
    echo "Error: File /dev/shm/sem.${SHARED_NAME}-data-produced exists!"
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

