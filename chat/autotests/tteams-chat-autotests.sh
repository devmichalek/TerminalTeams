#!/usr/bin/env bash

function exit-with-error() {
    echo "Error: Autotest $@ failed!"
    exit 1
}

function run-and-exit-on-error() {
    echo "Info: Running $@ ..."
    $@ || exit-with-error $@
}

run-and-exit-on-error ./tteams-chat-autotests-happy-path.sh

echo "Success: Autotests \"chat\" passed!"
exit 0
