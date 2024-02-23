### Usage
Application runs in Linux Terminal Emulator to:
- find users on LAN,
- communicate with other people on LAN

The tmux terminal multiplexer is used to create program interface.

### Assumptions
Application was written with the following assumptions:
- each username is unique within it's interface IP address
- each user has the same local time on the system
- only one user runs application per interface IP address

### Installation
```
cmake -B build
make all -j 4 -C build
cmake --install build --prefix install
```

### Debugging
Use Visual Studio Code (with C/C++ Extension Pack extension) to debug application.

### Technology stack
- C++
- CMake
- Google Protocol Buffers
- Google gRPC
- GoogleTest
- Bash
- IPC