#!/usr/bin/env bash

# Test scenario
# 1. Create dummy IPv4 addresses for dummy neighbors
# 2. Use dummy neighbors to communicate with engine
# 3. Check engine behavior

source ./tteams-engine-autotests-venv-activate.sh

# List IPv4 addresses for this interface
ip addr show dev eno1

# Add IPv4 address for this interface
sudo ip addr add 192.168.0.180/24 dev eno1
ip addr show dev eno1

./tteams-engine-autotests-dummy-neighbor.py --src-ip-address 192.168.0.180 --src-port 17888 --dst-ip-address 192.168.0.119 --dst-port 17888 --scenario 1.1

# Remove IPv4 address for this interface
sudo ip addr del 192.168.0.180/24 dev eno1
ip addr show dev eno1

source ./tteams-engine-autotests-venv-deactivate.sh
