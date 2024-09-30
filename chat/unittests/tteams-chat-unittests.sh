#!/usr/bin/env bash

if ! which valgrind &> /dev/null; then
    echo "Error: Failed to find valgrind on your system!"
    echo "Info: Please install valgrind for your system https://valgrind.org/"
    exit 1
fi

valgrind --tool=memcheck \
         --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --error-exitcode=1 \
         ./tteams-chat-unittests 2>&1 | tee -a tteams-chat-unittests-results.txt || exit 1
