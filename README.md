### Usage
Application runs in Linux Terminal Emulator to:
- find users on LAN,
- communicate with other people on LAN

The tmux terminal multiplexer is used to create program interface.

### Assumptions
Application was written with the following assumptions:
- each username is unique within it's interface IP address
- 

### Technology stack
- C++
- CMake
- Google Protocol Buffers
- Google gRPC
- GoogleTest
- Bash
- IPC

### Installation
```
cmake -B build
make all -j 4 -C build
cmake --install build --prefix install
```