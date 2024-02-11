Idea:
- Application should find users on LAN,
- Communicate with other people using app on LAN,

Usage:
- Android App
- Linux Terminal Emulator

Technology stack:
- C++
- CMake
- Makefile
- Google Protocol Buffers
- Google gRPC
- Java

How to build the project?
cmake -B ./build
make all -j -C ./build
cmake --install ./build --prefix .