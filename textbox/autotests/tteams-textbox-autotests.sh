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

echo "Success: Autotests \"textbox\" passed!"
exit 0
