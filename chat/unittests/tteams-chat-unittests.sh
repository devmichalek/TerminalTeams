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
         ./tteams-chat-ut &> unittests-results.txt
