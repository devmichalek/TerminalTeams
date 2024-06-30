#!/usr/bin/env bash

INPUT_FILENAME="$1"
OUTPUT_PATH="$2"
INCLUDE_PATH="$3"

if [ -z "$INPUT_FILENAME" ]
  then
    echo "Error: Input filename is not specified!"
    exit 1
fi

if [ -z "$OUTPUT_PATH" ]
  then
    echo "Error: Output directory is not specified!"
    exit 1
fi

if [ -z "$INCLUDE_PATH" ]
  then
    echo "Error: Include path not specified!"
    exit 1
fi

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

source "${SCRIPT_DIR}/tteams-engine-autotests-venv-activate.sh" "${OUTPUT_PATH}"

python -m grpc_tools.protoc -I${INCLUDE_PATH} --python_out=${OUTPUT_PATH} --pyi_out=${OUTPUT_PATH} --grpc_python_out=${OUTPUT_PATH} ${INPUT_FILENAME}

source "${SCRIPT_DIR}/tteams-engine-autotests-venv-deactivate.sh"

exit 0
