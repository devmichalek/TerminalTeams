### Usage
Application can be used in Linux Terminal Emulator to:
- find users on LAN,
- communicate with other people on LAN

### Technology stack
- C++
- CMake
- Makefile
- Google Protocol Buffers
- Google gRPC
- GoogleTest

### Installation
```
git submodule update --init --recursive
cmake -B build
make all -j 4 -C build
```