#!/usr/bin/env bash

INTERFACE1="tteams1"
INTERFACE2="tteams2"
INTERFACE3="tteams3"
INTERFACE4="tteams4"
MAC_ADDRESS1="cc:cc:cc:cc:cc:00"
MAC_ADDRESS2="dd:dd:dd:dd:dd:11"
MAC_ADDRESS3="ee:ee:ee:ee:ee:22"
MAC_ADDRESS4="ff:ff:ff:ff:ff:33"
IP_ADDRESS1="192.168.60.1"
IP_ADDRESS2="192.168.60.5"
IP_ADDRESS3="192.168.60.9"
IP_ADDRESS4="192.168.60.13"
IP_MASK=24

./tteams-interface-add.sh "${INTERFACE1}" $MAC_ADDRESS1 $IP_ADDRESS1 $IP_MASK
./tteams-interface-add.sh "${INTERFACE2}" $MAC_ADDRESS2 $IP_ADDRESS2 $IP_MASK
./tteams-interface-add.sh "${INTERFACE3}" $MAC_ADDRESS3 $IP_ADDRESS3 $IP_MASK
./tteams-interface-add.sh "${INTERFACE4}" $MAC_ADDRESS4 $IP_ADDRESS4 $IP_MASK

./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS3 $IP_ADDRESS3
./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS3 $IP_ADDRESS3
./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS3 $IP_ADDRESS3

read -p "Press enter to continue"

./tteams-interface-delete.sh "${INTERFACE1}"
./tteams-interface-delete.sh "${INTERFACE2}"
./tteams-interface-delete.sh "${INTERFACE3}"
./tteams-interface-delete.sh "${INTERFACE4}"
