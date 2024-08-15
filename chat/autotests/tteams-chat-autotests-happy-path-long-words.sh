#!/usr/bin/env bash

# Test setup
APP_HANDLER_STDIN=app-handler-stdin
APP_HANDLER_STDOUT=app-handler-stdout
APP_STDOUT=app-stdout
MSG_QUEUE_NAME=chat
APP_HANDLER="./tteams-chat-handler"
APP_HANDLER_ARGS=(40 10 "${MSG_QUEUE_NAME}")
APP_HANDLER_CMD="${APP_HANDLER} ${APP_HANDLER_ARGS[@]}"
APP="./tteams-chat"
APP_ARGS=(40 10 "${MSG_QUEUE_NAME}")
APP_CMD="${APP} ${APP_ARGS[@]}"
mkfifo ${APP_HANDLER_STDIN}
sleep infinity > ${APP_HANDLER_STDIN} &
APP_HANDLER_STDIN_PID=$!
${APP_HANDLER_CMD} < "${APP_HANDLER_STDIN}" &> "${APP_HANDLER_STDOUT}" &
${APP_CMD} &> "${APP_STDOUT}" &

# Test scenario
echo "Info: Waiting for synchronization..."
sleep 3
echo "create 0" > "${APP_HANDLER_STDIN}"
echo "create 1" > "${APP_HANDLER_STDIN}"
echo "select 0" > "${APP_HANDLER_STDIN}"
echo "send 0 Lorem ipsum dolor sit amet, consectetur adipiscing elit. Quisque leo nulla, consequat ut accumsan ac, sodales non lorem. Etiam at finibus sapien. Integer auctor nibh aliquam enim dapibus commodo. Pellentesque tempor, mi sit amet sodales convallis, sem neque ultrices eros, id sollicitudin velit turpis vel sapien. Vivamus quis leo ipsum. Nulla facilisi. Aliquam tincidunt orci eget lacus luctus, in tincidunt lorem mollis. Suspendisse potenti. Donec in semper ipsum." > "${APP_HANDLER_STDIN}"
echo "receive 0 Proin non tellus at augue aliquam vulputate. Maecenas fermentum nisl sem, a iaculis lectus consequat sit amet. Sed cursus pulvinar volutpat. Proin ut malesuada erat. Praesent aliquet ullamcorper odio, non molestie lorem luctus ut. Etiam libero nunc, mollis ac efficitur quis, iaculis id ex. Integer tristique, sapien at placerat sollicitudin, nisl felis viverra sapien, id egestas magna quam quis enim. Donec at dui non lacus venenatis vulputate ac id est." > "${APP_HANDLER_STDIN}"
echo "send 0 VeryLongMessageTakingMoreThanTotalSideWidthBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlah" > "${APP_HANDLER_STDIN}"
echo "receive 0 VeryLongMessageTakingMoreThanTotalSideWidthBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlahBlah" > "${APP_HANDLER_STDIN}"
echo "select 1" > "${APP_HANDLER_STDIN}"
echo "send 1 Pneumonoultramicroscopicsilicovolcanoconiosis, Pseudopseudohypoparathyroidism, Floccinaucinihilipilification, Antidisestablishmentarianism" > "${APP_HANDLER_STDIN}"
echo "receive 1 Supercalifragilisticexpialidocious, Strengths, Euouae, Unimaginatively, Honorificabilitudinitatibus, Tsktsk, Sesquipedalianism, Uncopyrightable" > "${APP_HANDLER_STDIN}"
echo "select 0" > "${APP_HANDLER_STDIN}"
echo "Info: Waiting for data to be set..."
sleep 3

# Expected output
APP_EXPECTED_RESULTS_RAW="                     1970-01-01 01:00:00
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
APP_EXPECTED_RESULTS=$(echo -e "$APP_EXPECTED_RESULTS_RAW")
APP_ACTUAL_RESULTS=$(<"${APP_STDOUT}")
APP_HANDLER_EXPECTED_RESULTS="create status=1
create status=1
select status=1
send status=1
receive status=1
send status=1
receive status=1
select status=1
send status=1
receive status=1
select status=1"
APP_HANDLER_ACTUAL_RESULTS=$(<"${APP_HANDLER_STDOUT}")

# Test teardown
kill $APP_HANDLER_STDIN_PID
echo "Info: Waiting for application to stop..."
sleep 3
echo "Info: Application shall be stopped now"
source tteams-chat-autotests-verdict.sh