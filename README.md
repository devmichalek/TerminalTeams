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
- no centralized server
- no support for IPv6
- terminal emulator window resizing is not supported

### Installation
```
cmake -B build
make all -j 4 -C build
cmake --install build --prefix install
```

### Technology stack
- C++ Multithreading
- CMake
- Google Protocol Buffers
- Google gRPC
- GoogleTest
- Bash
- Inter-process communication
- Virtualization
- Networking

### Debugging
Use Visual Studio Code (with C/C++ Extension Pack extension) to debug application.

### Testing
Application was tested using:
- Unit tests,
- Manual tests - using up to four VMs running together in a virtual network.