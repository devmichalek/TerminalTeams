### Usage
Application runs in Linux Terminal Emulator to:
- find users on LAN,
- chat with other people on LAN

The tmux terminal multiplexer is used to create program interface.

### Assumptions
Application was written with the following assumptions:
- each username is unique within it's interface IP address
- each user has the same local time on the system
- only one user runs application per IP address interface
- no centralized server
- no support for IPv6
- terminal emulator window resizing is not supported

### Installation
```
cmake -B build
cmake --build build
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
- [VirtualBox](https://www.virtualbox.org/)
- Networking
- [PlantUML](https://valgrind.org/)
- [Valgrind](https://valgrind.org/)
- [Linux Trace Toolkit (next generation)](https://lttng.org/) and [Babeltrace](https://babeltrace.org/)

### Debugging
Use Visual Studio Code (with C/C++ Extension Pack extension) to debug application.

### Testing
Application was tested using:
- Unit tests,
- Integration tests,
- Manual tests - using up to four VMs running together in a virtual network.

### Todo
- use std::shared_mutex
- user + password
- [[fallthrough]]
- [[noreturn]]
- [[likely]]
- std::launder
- std::once_flag
- safe cleanup
- use LTTng
