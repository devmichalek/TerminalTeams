#!/usr/bin/env bash

function exit-with-error() {
    echo "Error: Autotests $@ failed!"
    exit 1
}

echo "Info: Running all autotests..."

./tteams-chat-autotests.sh || exit-with-error "\"chat\""
./tteams-contacts-autotests.sh || exit-with-error "\"contacts\""
./tteams-textbox-autotests.sh || exit-with-error "\"textbox\""

echo "Success: All autotests passed!"
exit 0
