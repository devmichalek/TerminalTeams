#!/usr/bin/env bash

function exit-with-error() {
    echo "Error: Autotest $@ failed!"
    exit 1
}

function run-and-exit-on-error() {
    echo "Info: Running $@ ..."
    $@ || exit-with-error $@
}

echo "Info: Running \"textbox\" autotests..."

run-and-exit-on-error ./tteams-textbox-autotests-happy-path.sh
run-and-exit-on-error ./tteams-textbox-autotests-happy-path-help.sh
run-and-exit-on-error ./tteams-textbox-autotests-happy-path-message.sh
run-and-exit-on-error ./tteams-textbox-autotests-happy-path-quit.sh
run-and-exit-on-error ./tteams-textbox-autotests-happy-path-select.sh
run-and-exit-on-error ./tteams-textbox-autotests-unhappy-path.sh
run-and-exit-on-error ./tteams-textbox-autotests-unhappy-path-help.sh
run-and-exit-on-error ./tteams-textbox-autotests-unhappy-path-kill.sh
run-and-exit-on-error ./tteams-textbox-autotests-unhappy-path-quit.sh
run-and-exit-on-error ./tteams-textbox-autotests-unhappy-path-select.sh

echo "Success: Autotests \"textbox\" passed!"
exit 0
