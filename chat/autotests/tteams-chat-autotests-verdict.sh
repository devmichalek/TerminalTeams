#!/usr/bin/env bash

EXIT_STATUS=0
APP_PID=$(pgrep -f "${APP_CMD}" | head -n 1)
if [[ "${APP_PID}" ]]; then
    echo "Error: Application is still running! Killing pid=$APP_PID..."
    kill -9 $APP_PID
    EXIT_STATUS=1
fi
APP_HANDLER_PID=$(pgrep -f "${APP_HANDLER_CMD}" | head -n 1)
if [[ "${APP_HANDLER_PID}" ]]; then
    echo "Error: Application handler is still running! Killing pid=$APP_HANDLER_PID..."
    kill -9 $APP_HANDLER_PID
    EXIT_STATUS=1
fi
MSG_QUEUE_PRIMARY_PATH="/dev/mqueue/${MSG_QUEUE_NAME}-primary"
if [ -f "${MSG_QUEUE_PRIMARY_PATH}" ]; then
    echo "Error: File ${MSG_QUEUE_PRIMARY_PATH} exists! Removing this file..."
    rm -f "${MSG_QUEUE_PRIMARY_PATH}"
    EXIT_STATUS=1
fi
MSG_QUEUE_SECONDARY_PATH="/dev/mqueue/${MSG_QUEUE_NAME}-secondary"
if [ -f "${MSG_QUEUE_SECONDARY_PATH}" ]; then
    echo "Error: File ${MSG_QUEUE_SECONDARY_PATH} exists! Removing this file..."
    rm -f "${MSG_QUEUE_SECONDARY_PATH}"
    EXIT_STATUS=1
fi
if [[ "$APP_ACTUAL_RESULTS" != "$APP_EXPECTED_RESULTS" ]]; then
    echo "Error: Actual app results are different than expected!"
    printf "%s" "$APP_ACTUAL_RESULTS" > app_actual_results.txt
    printf "%s" "$APP_EXPECTED_RESULTS" > app_expected_results.txt
    EXIT_STATUS=1
fi
if [[ "$APP_HANDLER_ACTUAL_RESULTS" != "$APP_HANDLER_EXPECTED_RESULTS" ]]; then
    echo "Error: Actual app handler results are different than expected!"
    printf "%s" "$APP_HANDLER_ACTUAL_RESULTS" > app_handler_actual_results.txt
    printf "%s" "$APP_HANDLER_EXPECTED_RESULTS" > app_handler_expected_results.txt
    EXIT_STATUS=1
fi
if [[ $EXIT_STATUS -eq 0 ]]; then
    echo "Success: Autotest passed!"
    rm -f "${APP_HANDLER_STDIN}"
    rm -f "${APP_HANDLER_STDOUT}"
    rm -f "${APP_STDOUT}"
fi
exit $EXIT_STATUS