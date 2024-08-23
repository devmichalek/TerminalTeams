#!/usr/bin/env bash

function exit-with-error() {
    echo "Error: Unittests $@ failed!"
    exit 1
}

./tteams-chat-unittests.sh || exit-with-error "\"chat\""
./tteams-contacts-unittests.sh || exit-with-error "\"contacts\""
./tteams-textbox-unittests.sh || exit-with-error "\"textbox\""
./tteams-engine-unittests.sh || exit-with-error "\"engine\""

echo "Success: All unittests passed!"
exit 0
