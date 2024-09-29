#!/usr/bin/env bash

function exit-with-error() {
    echo "Error: Autotest $@ failed!"
    exit 1
}

function run-and-exit-on-error() {
    echo "Info: Running $@ ..."
    $@ || exit-with-error $@
}

echo "Info: Running \"contacts\" autotests..."

run-and-exit-on-error ./tteams-contacts-autotests-happy-path-select.sh
run-and-exit-on-error ./tteams-contacts-autotests-happy-path-send-receive.sh
run-and-exit-on-error ./tteams-contacts-autotests-happy-path-active-inactive.sh
run-and-exit-on-error ./tteams-contacts-autotests-unhappy-path-send-receive.sh
run-and-exit-on-error ./tteams-contacts-autotests-unhappy-path-non-existing.sh

echo "Success: Autotests \"contacts\" passed!"
exit 0
