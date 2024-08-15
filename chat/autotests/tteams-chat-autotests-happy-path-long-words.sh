#!/usr/bin/env bash

# Test setup
HANDLER_STDIN=handler-stdin
HANDLER_STDOUT=handler-stdout
MSG_QUEUE_NAME=chat
APP_HANDLER="./tteams-chat-handler"
APP_HANDLER_ARGS=(40 10 "${MSG_QUEUE_NAME}")
APP_HANDLER_CMD="${APP_HANDLER} ${APP_HANDLER_ARGS[@]}"
APP="./tteams-chat"
APP_ARGS=(40 10 "${MSG_QUEUE_NAME}")
APP_CMD="${APP} ${APP_ARGS[@]}"
mkfifo ${HANDLER_STDIN}
sleep infinity > ${HANDLER_STDIN} &
HANDLER_STDIN_PID=$!
${APP_HANDLER_CMD} < "${HANDLER_STDIN}" &
${APP_CMD} &> "${HANDLER_STDOUT}" &

# Test scenario
echo "Info: Waiting for synchronization..."
sleep 3
echo "create 0" > "${HANDLER_STDIN}"
echo "create 1" > "${HANDLER_STDIN}"
echo "select 0" > "${HANDLER_STDIN}"
echo "send 0 Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque leo nulla, consequat ut accumsan ac, sodales non lorem. Etiam at finibus sapien. Integer auctor nibh aliquam enim dapibus commodo. Pellentesque tempor, mi sit amet sodales convallis, sem neque ultrices eros, id sollicitudin velit turpis vel sapien. Vivamus quis leo ipsum. Nulla facilisi. Aliquam tincidunt orci eget lacus luctus, in tincidunt lorem mollis. Suspendisse potenti. Donec in semper ipsum." > "${HANDLER_STDIN}"
echo "receive 0 Proin non tellus at augue aliquam vulputate. Maecenas fermentum nisl sem, a iaculis lectus consequat sit amet. Sed cursus pulvinar volutpat. Proin ut malesuada erat. Praesent aliquet ullamcorper odio, non molestie lorem luctus ut. Etiam libero nunc, mollis ac efficitur quis, iaculis id ex. Integer tristique, sapien at placerat sollicitudin, nisl felis viverra sapien, id egestas magna quam quis enim. Donec at dui non lacus venenatis vulputate ac id est." > "${HANDLER_STDIN}"
echo "send 0 VeryLongMessageTakingMoreThanTotalSideWidthBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlah" > "${HANDLER_STDIN}"
echo "receive 0 VeryLongMessageTakingMoreThanTotalSideWidthBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlah" > "${HANDLER_STDIN}"
echo "select 1" > "${HANDLER_STDIN}"
echo "send 1 Pneumonoultramicroscopicsilicovolcanoconiosis, Pseudopseudohypoparathyroidism, Floccinaucinihilipilification, Antidisestablishmentarianism" > "${HANDLER_STDIN}"
echo "receive 1 Supercalifragilisticexpialidocious, Strengths, Euouae, Unimaginatively, Honorificabilitudinitatibus, Tsktsk, Sesquipedalianism, Uncopyrightable" > "${HANDLER_STDIN}"
echo "select 0" > "${HANDLER_STDIN}"
echo "Info: Waiting for data to be set..."
sleep 3

# Expected output
EXPECTED_RESULTS_RAW="                     1970-01-01 01:00:00
             Lorem ipsum dolor sit amet,
            consectetur adipiscing elit.
            Quisque leo nulla, consequat
             ut accumsan ac, sodales non
                 lorem. Etiam at finibus
             sapien. Integer auctor nibh
                    aliquam enim dapibus
                   commodo. Pellentesque
             tempor, mi sit amet sodales
                    convallis, sem neque
                       ultrices eros, id
               sollicitudin velit turpis
            vel sapien. Vivamus quis leo
                  ipsum. Nulla facilisi.
             Aliquam tincidunt orci eget
              lacus luctus, in tincidunt
               lorem mollis. Suspendisse
                potenti. Donec in semper
                                  ipsum.

1970-01-01 01:00:00
Proin non tellus at augue
aliquam vulputate. Maecenas
fermentum nisl sem, a
iaculis lectus consequat sit
amet. Sed cursus pulvinar
volutpat. Proin ut malesuada
erat. Praesent aliquet
ullamcorper odio, non
molestie lorem luctus ut.
Etiam libero nunc, mollis ac
efficitur quis, iaculis id
ex. Integer tristique,
sapien at placerat
sollicitudin, nisl felis
viverra sapien, id egestas
magna quam quis enim. Donec
at dui non lacus venenatis
vulputate ac id est.

                     1970-01-01 01:00:00
            VeryLongMessageTakingMoreTha
            nTotalSideWidthBlahBlahBlahB
            lahBlahBlahBlahBlahBlahBlahB
            lahBlahBlahBlahBlahBlahBlahB
            lahBlahBlahBlahBlahBlahBlahB
                                 lahBlah

1970-01-01 01:00:00
VeryLongMessageTakingMoreTha
nTotalSideWidthBlahBlahBlahB
lahBlahBlahBlahBlahBlahBlahB
lahBlahBlahBlahBlahBlahBlahB
lahBlahBlahBlahBlahBlahBlahB
lahBlah

\033[2J\033[1;1H                     1970-01-01 01:00:00
            Pneumonoultramicroscopicsili
                      covolcanoconiosis,
            Pseudopseudohypoparathyroidi
                                     sm,
            Floccinaucinihilipilificatio
                                      n,
            Antidisestablishmentarianism
                                        

1970-01-01 01:00:00
Supercalifragilisticexpialid
ocious, Strengths, Euouae,
Unimaginatively,
Honorificabilitudinitatibus,
 Tsktsk, Sesquipedalianism,
Uncopyrightable

\033[2J\033[1;1H                     1970-01-01 01:00:00
             Lorem ipsum dolor sit amet,
            consectetur adipiscing elit.
            Quisque leo nulla, consequat
             ut accumsan ac, sodales non
                 lorem. Etiam at finibus
             sapien. Integer auctor nibh
                    aliquam enim dapibus
                   commodo. Pellentesque
             tempor, mi sit amet sodales
                    convallis, sem neque
                       ultrices eros, id
               sollicitudin velit turpis
            vel sapien. Vivamus quis leo
                  ipsum. Nulla facilisi.
             Aliquam tincidunt orci eget
              lacus luctus, in tincidunt
               lorem mollis. Suspendisse
                potenti. Donec in semper
                                  ipsum.

1970-01-01 01:00:00
Proin non tellus at augue
aliquam vulputate. Maecenas
fermentum nisl sem, a
iaculis lectus consequat sit
amet. Sed cursus pulvinar
volutpat. Proin ut malesuada
erat. Praesent aliquet
ullamcorper odio, non
molestie lorem luctus ut.
Etiam libero nunc, mollis ac
efficitur quis, iaculis id
ex. Integer tristique,
sapien at placerat
sollicitudin, nisl felis
viverra sapien, id egestas
magna quam quis enim. Donec
at dui non lacus venenatis
vulputate ac id est.

                     1970-01-01 01:00:00
            VeryLongMessageTakingMoreTha
            nTotalSideWidthBlahBlahBlahB
            lahBlahBlahBlahBlahBlahBlahB
            lahBlahBlahBlahBlahBlahBlahB
            lahBlahBlahBlahBlahBlahBlahB
                                 lahBlah

1970-01-01 01:00:00
VeryLongMessageTakingMoreTha
nTotalSideWidthBlahBlahBlahB
lahBlahBlahBlahBlahBlahBlahB
lahBlahBlahBlahBlahBlahBlahB
lahBlahBlahBlahBlahBlahBlahB
lahBlah"
EXPECTED_RESULTS=$(echo -e "$EXPECTED_RESULTS_RAW")
ACTUAL_RESULTS=$(<"${HANDLER_STDOUT}")

# Test teardown
kill $HANDLER_STDIN_PID
echo "Info: Waiting for application to stop..."
sleep 3
echo "Info: Application shall be stopped now"
source tteams-chat-autotests-verdict.sh