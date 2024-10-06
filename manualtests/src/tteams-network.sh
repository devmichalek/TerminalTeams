#!/usr/bin/env bash

INTERFACE1="tteams1"
INTERFACE2="tteams2"
INTERFACE3="tteams3"
INTERFACE4="tteams4"
INTERFACE5="tteams5"
INTERFACE6="tteams6"
INTERFACE7="tteams7"
INTERFACE8="tteams8"
INTERFACE9="tteams9"
MAC_ADDRESS1="cc:cc:cc:cc:cc:00"
MAC_ADDRESS2="dd:dd:dd:dd:dd:11"
MAC_ADDRESS3="ee:ee:ee:ee:ee:22"
MAC_ADDRESS4="ff:ff:ff:ff:ff:33"
MAC_ADDRESS5="aa:ff:aa:ff:aa:44"
MAC_ADDRESS6="ff:bb:ff:bb:ff:55"
MAC_ADDRESS7="cc:ff:cc:ff:cc:66"
MAC_ADDRESS8="ff:dd:ff:dd:ff:77"
MAC_ADDRESS9="ee:ff:ee:ff:ee:88"
IP_ADDRESS1="192.168.60.1"
IP_ADDRESS2="192.168.60.5"
IP_ADDRESS3="192.168.60.9"
IP_ADDRESS4="192.168.60.13"
IP_ADDRESS5="192.168.60.17"
IP_ADDRESS6="192.168.60.21"
IP_ADDRESS7="192.168.60.25"
IP_ADDRESS8="192.168.60.29"
IP_ADDRESS9="192.168.60.33"
IP_MASK=24

./tteams-interface-add.sh "${INTERFACE1}" $MAC_ADDRESS1 $IP_ADDRESS1 $IP_MASK
./tteams-interface-add.sh "${INTERFACE2}" $MAC_ADDRESS2 $IP_ADDRESS2 $IP_MASK
./tteams-interface-add.sh "${INTERFACE3}" $MAC_ADDRESS3 $IP_ADDRESS3 $IP_MASK
./tteams-interface-add.sh "${INTERFACE4}" $MAC_ADDRESS4 $IP_ADDRESS4 $IP_MASK
./tteams-interface-add.sh "${INTERFACE5}" $MAC_ADDRESS5 $IP_ADDRESS5 $IP_MASK
./tteams-interface-add.sh "${INTERFACE6}" $MAC_ADDRESS6 $IP_ADDRESS6 $IP_MASK
./tteams-interface-add.sh "${INTERFACE7}" $MAC_ADDRESS7 $IP_ADDRESS7 $IP_MASK
./tteams-interface-add.sh "${INTERFACE8}" $MAC_ADDRESS8 $IP_ADDRESS8 $IP_MASK
./tteams-interface-add.sh "${INTERFACE9}" $MAC_ADDRESS9 $IP_ADDRESS9 $IP_MASK

./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS3 $IP_ADDRESS3
./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS5 $IP_ADDRESS5
./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS6 $IP_ADDRESS6
./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS7 $IP_ADDRESS7
./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS8 $IP_ADDRESS8
./tteams-interface-add-neighbor.sh "${INTERFACE1}" $MAC_ADDRESS9 $IP_ADDRESS9


./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS3 $IP_ADDRESS3
./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS5 $IP_ADDRESS5
./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS6 $IP_ADDRESS6
./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS7 $IP_ADDRESS7
./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS8 $IP_ADDRESS8
./tteams-interface-add-neighbor.sh "${INTERFACE2}" $MAC_ADDRESS9 $IP_ADDRESS9

./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS5 $IP_ADDRESS5
./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS6 $IP_ADDRESS6
./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS7 $IP_ADDRESS7
./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS8 $IP_ADDRESS8
./tteams-interface-add-neighbor.sh "${INTERFACE3}" $MAC_ADDRESS9 $IP_ADDRESS9

./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS3 $IP_ADDRESS3
./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS5 $IP_ADDRESS5
./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS6 $IP_ADDRESS6
./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS7 $IP_ADDRESS7
./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS8 $IP_ADDRESS8
./tteams-interface-add-neighbor.sh "${INTERFACE4}" $MAC_ADDRESS9 $IP_ADDRESS9

./tteams-interface-add-neighbor.sh "${INTERFACE5}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE5}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE5}" $MAC_ADDRESS3 $IP_ADDRESS3
./tteams-interface-add-neighbor.sh "${INTERFACE5}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE5}" $MAC_ADDRESS6 $IP_ADDRESS6
./tteams-interface-add-neighbor.sh "${INTERFACE5}" $MAC_ADDRESS7 $IP_ADDRESS7
./tteams-interface-add-neighbor.sh "${INTERFACE5}" $MAC_ADDRESS8 $IP_ADDRESS8
./tteams-interface-add-neighbor.sh "${INTERFACE5}" $MAC_ADDRESS9 $IP_ADDRESS9

./tteams-interface-add-neighbor.sh "${INTERFACE6}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE6}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE6}" $MAC_ADDRESS3 $IP_ADDRESS3
./tteams-interface-add-neighbor.sh "${INTERFACE6}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE6}" $MAC_ADDRESS5 $IP_ADDRESS5
./tteams-interface-add-neighbor.sh "${INTERFACE6}" $MAC_ADDRESS7 $IP_ADDRESS7
./tteams-interface-add-neighbor.sh "${INTERFACE6}" $MAC_ADDRESS8 $IP_ADDRESS8
./tteams-interface-add-neighbor.sh "${INTERFACE6}" $MAC_ADDRESS9 $IP_ADDRESS9

./tteams-interface-add-neighbor.sh "${INTERFACE7}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE7}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE7}" $MAC_ADDRESS3 $IP_ADDRESS3
./tteams-interface-add-neighbor.sh "${INTERFACE7}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE7}" $MAC_ADDRESS5 $IP_ADDRESS5
./tteams-interface-add-neighbor.sh "${INTERFACE7}" $MAC_ADDRESS6 $IP_ADDRESS6
./tteams-interface-add-neighbor.sh "${INTERFACE7}" $MAC_ADDRESS8 $IP_ADDRESS8
./tteams-interface-add-neighbor.sh "${INTERFACE7}" $MAC_ADDRESS9 $IP_ADDRESS9

./tteams-interface-add-neighbor.sh "${INTERFACE8}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE8}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE8}" $MAC_ADDRESS3 $IP_ADDRESS3
./tteams-interface-add-neighbor.sh "${INTERFACE8}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE8}" $MAC_ADDRESS5 $IP_ADDRESS5
./tteams-interface-add-neighbor.sh "${INTERFACE8}" $MAC_ADDRESS6 $IP_ADDRESS6
./tteams-interface-add-neighbor.sh "${INTERFACE8}" $MAC_ADDRESS7 $IP_ADDRESS7
./tteams-interface-add-neighbor.sh "${INTERFACE8}" $MAC_ADDRESS9 $IP_ADDRESS9

./tteams-interface-add-neighbor.sh "${INTERFACE9}" $MAC_ADDRESS1 $IP_ADDRESS1
./tteams-interface-add-neighbor.sh "${INTERFACE9}" $MAC_ADDRESS2 $IP_ADDRESS2
./tteams-interface-add-neighbor.sh "${INTERFACE9}" $MAC_ADDRESS3 $IP_ADDRESS3
./tteams-interface-add-neighbor.sh "${INTERFACE9}" $MAC_ADDRESS4 $IP_ADDRESS4
./tteams-interface-add-neighbor.sh "${INTERFACE9}" $MAC_ADDRESS5 $IP_ADDRESS5
./tteams-interface-add-neighbor.sh "${INTERFACE9}" $MAC_ADDRESS6 $IP_ADDRESS6
./tteams-interface-add-neighbor.sh "${INTERFACE9}" $MAC_ADDRESS7 $IP_ADDRESS7
./tteams-interface-add-neighbor.sh "${INTERFACE9}" $MAC_ADDRESS8 $IP_ADDRESS8

read -p "Press enter to continue"

./tteams-interface-delete.sh "${INTERFACE1}"
./tteams-interface-delete.sh "${INTERFACE2}"
./tteams-interface-delete.sh "${INTERFACE3}"
./tteams-interface-delete.sh "${INTERFACE4}"
./tteams-interface-delete.sh "${INTERFACE5}"
./tteams-interface-delete.sh "${INTERFACE6}"
./tteams-interface-delete.sh "${INTERFACE7}"
./tteams-interface-delete.sh "${INTERFACE8}"
./tteams-interface-delete.sh "${INTERFACE9}"
