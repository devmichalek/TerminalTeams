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
- no support for IPv6
- terminal emulator window resizing is not supported
- ASCII is the only supported set of characters

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
- [Google GRPC](https://grpc.io/)
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

### Testing
Application was tested using:
- Unit tests,
- Integration tests - so called auto tests,
- E2E tests - using up to three docker containers and virtual network,
- Manual tests - using up to three VMs and virtual network

### Future extensions and support
- support for other character encodings

### Todo
- [[fallthrough]]
- [[noreturn]]
- [[likely]]
- [[nodiscard]]
- std::launder
- std::once_flag
- safe cleanup
- TTEngineTest.cpp unit tests
- TTBroadcasterDiscoveryTest.cpp unit tests
- TTHeartbeatRequest and other, add operators and constructors and cout operators
- utils unit tests
- engine auto tests
- TTConfig.hpp for each module displayed in logs
- engine documentation
- remove IPC files in cpp classes in constructor
- comment code
