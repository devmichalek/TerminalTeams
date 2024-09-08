# Build
FROM alpine:3.17.0 AS build

RUN apk update && \
    apk add --no-cache \
        make=4.3-r1 \
        g++=12.2.1_git20220924-r4 \
        cmake=3.24.4-r0 \
        git=2.38.5-r0 \
        linux-headers=5.19.5-r0

WORKDIR /tteams

COPY ./chat ./chat
COPY ./contacts ./contacts
COPY ./diagnostics ./diagnostics
COPY ./e2etests ./e2etests
COPY ./engine ./engine
COPY ./textbox ./textbox
COPY ./unittests ./unittests
COPY ./utils ./utils
COPY ./CMakeLists.txt .
COPY ./README.md .
RUN ls -la /tteams

RUN ls -la /tteams && cmake -B build && \
    cmake --build build --parallel 4 && \
    cmake --install build --prefix install

# Runtime
FROM alpine:3.17.0

RUN apk update && \
    apk add --no-cache \
        tmux=3.3a-r0

RUN addgroup -S tteamssuser && adduser -S tteamssuser -G tteamssuser
USER tteamssuser

COPY --chown=tteamssuser:tteamssuser --from=build /tteams/install/bin ./app/

ENTRYPOINT [ "./app/bin/tteams.sh" ]
