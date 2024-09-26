# Terminal Teams Engine
## About
Engine is the key module of Terminal Teams. Engine handles LAN communication using gRPC and protocol buffers through services. This module ensures proper communication across other modules, connects services with handlers and is responsible for global synchronization. Engine works as a backbone backend service thus there is no user interfaces to interact with.

## Architecture
This module consist of one main component that implements:
- services - so called gRPC services for neighbors discovery and neighbors chat
- broadcasters - inbound message handlers
- stub - so called gRPC stub, outbound message handler

![TTEngine](./doc/TTEngine.svg)

## Communication

![TTEngineCommunication](./doc/TTEngineCommunication.svg)
