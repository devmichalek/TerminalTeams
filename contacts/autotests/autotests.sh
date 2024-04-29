#!/usr/bin/env bash

function exit-with-error() {
    echo "Error: Autotest $@ failed!"
    exit 1
}

./autotests-happy-path.sh || exit-with-error "\"happy path\""

echo "Success: All autotests passed!"
exit 0
