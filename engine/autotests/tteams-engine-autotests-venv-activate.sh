#!/usr/bin/env bash

if ! which python3 &> /dev/null; then
    echo "Error: Failed to find required Python 3!"
    exit 1
fi

if ! python3 -m pip --version &> /dev/null; then
    echo "Error: Failed to find Python 3 pip module!"
    exit 1
fi

if [ -z "$1" ]
  then
    echo "Error: Output directory is not specified!"
    exit 1
fi

python3 -m venv "${1}/venv"
source "${1}/venv/bin/activate"
python3 -m pip install grpcio
python3 -m pip install grpcio-tools
