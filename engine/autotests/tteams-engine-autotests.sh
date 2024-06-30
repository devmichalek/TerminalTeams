#!/usr/bin/env bash

function exit-with-error() {
    echo "Error: Autotest $@ failed!"
    exit 1
}

function run-and-exit-on-error() {
    echo "Info: Running $@ ..."
    $@ || exit-with-error $@
}

echo "Info: Running \"engine\" autotests..."

run-and-exit-on-error ./tteams-engine-autotests-happy-path.sh

echo "Success: Autotests \"engine\" passed!"
exit 0
