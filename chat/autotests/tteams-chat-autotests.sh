#!/usr/bin/env bash

function exit-with-error() {
    echo "Error: Autotest $@ failed!"
    exit 1
}

function run-and-exit-on-error() {
    echo "Info: Running $@ ..."
    $@ || exit-with-error $@
}

echo "Info: Running \"chat\" autotests..."

run-and-exit-on-error ./tteams-chat-autotests-happy-path-additional-whitespaces.sh
run-and-exit-on-error ./tteams-chat-autotests-happy-path-duplicated-whitespaces.sh
run-and-exit-on-error ./tteams-chat-autotests-happy-path-long-words.sh
run-and-exit-on-error ./tteams-chat-autotests-happy-path.sh
run-and-exit-on-error ./tteams-chat-autotests-unhappy-path.sh

echo "Success: Autotests \"chat\" passed!"
exit 0
