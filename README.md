### Usage
Application runs inside Linux Terminal Emulator to:
- find users on LAN,
- chat with other people on LAN

The tmux terminal multiplexer was used to create program interface.

### Assumptions
Application was written with the following assumptions:
- each username is unique per PC's network interface
- each user in the LAN has the same local time on the system
- only one user can run an application per IP address interface
- no centralized server
- no support for IPv6 and TLS
- terminal emulator window resizing is not supported
- ASCII set is the only supported set of characters
- messages are ephemeral, no storage

### Installation
```
cmake -B build
cmake --build build --parallel 4
cmake --install build --prefix install
```

### Used frameworks, tools and concepts
- C++ Multithreading
- [CMake](https://cmake.org/)
- [Google Protocol Buffers](https://protobuf.dev/)
- [Google gRPC](https://grpc.io/)
- [Google Test](https://google.github.io/googletest/)
- Bash
- Inter-Process Communication
- [Docker](https://www.docker.com/)
- [VirtualBox](https://www.virtualbox.org/)
- Networking
- [PlantUML](https://valgrind.org/)
- [Valgrind](https://valgrind.org/)
- [Minitrace](https://github.com/hrydgard/minitrace)
- [Spdlog](https://github.com/gabime/spdlog)

### Debugging
Use Visual Studio Code (with C/C++ Extension Pack extension) to debug unit tests. Specify `-DCMAKE_BUILD_TYPE=Debug` to generate [trace](chrome://tracing/) and log files in `/tmp` directory on runtime.

### Test strategy
Application was tested using:
- Unit tests,
- Integration tests - so called auto tests,
- E2E tests - using up to three docker containers and virtual networks,
- Manual tests - using up to three VMs and virtual networks

### Future extensions and support
- support for other character encodings
- support for IPv6 and TLS
- messages rollback after shutdown based on trusted peer (requires TLS)

### Todo
- std::launder
- safe cleanup
- utils unit tests
- engine auto tests
- engine documentation
- remove IPC files in cpp classes in constructor
- comment code
- fix VerifyApplicationTimeout
- add to TTUtilsStopable a subscribe method, condition_variable etc.
- when TTTextBoxHandler is closed with kill, the TTTextBox is not deleteding named pipe
- reject messages to itself
