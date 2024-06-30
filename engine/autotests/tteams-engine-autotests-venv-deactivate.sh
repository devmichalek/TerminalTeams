#!/usr/bin/env bash

if ! which python3 &> /dev/null; then
    echo "Error: Failed to find required Python 3!"
    exit 1
fi

if ! type deactivate &> /dev/null; then
    echo "Error: Failed to find active virtual environment!"
    exit 1
fi

deactivate
