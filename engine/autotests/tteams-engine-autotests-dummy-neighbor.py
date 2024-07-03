#!/usr/bin/env python3

# Top-level functionality
# 1. Replying with HeartbeatReply
# 2. Requesting with HeartbeatRequest
# 3. Replying with GreetReply
# 4. Requesting with GreetRequest
# 5. Replying with TellReply
# 6. Requesting with TellRequest
# 7. Replying with NarrateReply
# 8. Requesting with NarrateRequest

# Scenarios
# 1.1 Always reply with HeartbeatReply on HeartbeatRequest
# 1.2 Reply with HeartbeatReply n times on HeartbeatReqest then stop replying
# 2.1 Request with HeartbeatRequest every t seconds
# 3.1 Always reply with GreetReply on GreetRequest
# 4.1 Request with GreetRequest n times every t seconds until first GreetReply
# 5.1 Always reply with TellReply on TellRequest
# 5.2 Reply with TellReply every second time for particular TellRequest sequence number
# 6.1 Request with TellRequest every t seconds, up to n messages
# 7.1 Always reply with NarrateReply on NarrateReply
# 7.2 Reply with NarratReply every second time for particular NarrateReply sequence number
# 8.1 Request with NarrateRequest once

import argparse
import logging
import grpc
import concurrent
import time
import signal
import threading
import TerminalTeams_pb2
import TerminalTeams_pb2_grpc

logger = logging.getLogger(__name__)

def scenario_1_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 1.1...")
    server_shutdown_event = threading.Event()
    def stop_server(signum, frame):
        server_shutdown_event.set()
    class Servicer(TerminalTeams_pb2_grpc.NeighborsDiscovery):
        def Heartbeat(self, request, context):
            logging.info("Sending dummy HeartbeatReply...")
            return TerminalTeams_pb2.HeartbeatReply(identity="identity")
    signal.signal(signal.SIGTERM, stop_server)
    server = grpc.server(concurrent.futures.ThreadPoolExecutor(max_workers=1))
    TerminalTeams_pb2_grpc.add_NeighborsDiscoveryServicer_to_server(Servicer(), server)
    server.add_insecure_port(src_ip_address + ":" + src_port)
    server.start()
    logging.info("Server started, listening on " + src_ip_address + ":" + src_port)
    server_shutdown_event.wait()
    server.stop(3).wait()

def scenario_1_2(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 1.2...")
    heartbeat_counter = 0
    heartbeat_limit = 10
    server_shutdown_event = threading.Event()
    def stop_server(signum, frame):
        server_shutdown_event.set()
    def count_heartbeat():
        heartbeat_counter += 1
        if heartbeat_counter >= heartbeat_limit:
            stop_server(0, 0)
    class Servicer(TerminalTeams_pb2_grpc.NeighborsDiscovery):
        def Heartbeat(self, request, context):
            count_heartbeat()
            logging.info("Sending dummy HeartbeatReply...")
            return TerminalTeams_pb2.HeartbeatReply(identity="identity")
    signal.signal(signal.SIGTERM, stop_server)
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=1))
    TerminalTeams_pb2_grpc.add_NeighborsDiscoveryServicer_to_server(Servicer(), server)
    server.add_insecure_port(src_ip_address + ":" + src_port)
    server.start()
    logging.info("Server started, listening on " + src_ip_address + ":" + src_port)
    server_shutdown_event.wait()
    server.stop(3).wait()

def scenario_2_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 2.1...")
    client_shutdown_event = threading.Event()
    def stop_client(signum, frame):
        server_shutdown_event.set()
    signal.signal(signal.SIGTERM, stop_client)
    with grpc.insecure_channel(dst_ip_address + ":" + dst_port) as channel:
        stub = TerminalTeams_pb2_grpc.NeighborsDiscoveryStub(channel)
        while not client_shutdown_event.is_set():
            try:
                logging.info("Sending dummy HeartbeatRequest...")
                stub.Heartbeat(TerminalTeams_pb2.HeartbeatRequest(identity="identity"))
            except Exception:
                logging.error("Failed to send dummy HeartbeatRequest!")
                pass
            time.sleep(2)
    client_shutdown_event.wait()

def scenario_3_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 3.1...")
    server_shutdown_event = threading.Event()
    def stop_server(signum, frame):
        server_shutdown_event.set()
    class Servicer(TerminalTeams_pb2_grpc.NeighborsDiscovery):
        def Heartbeat(self, request, context):
            logging.info("Sending dummy GreetReply...")
            return TerminalTeams_pb2.GreetReply(nickname="nickname", identity="identity", ipAddressAndPort=src_ip_address + ":" + src_port)
    signal.signal(signal.SIGTERM, stop_server)
    server = grpc.server(concurrent.futures.ThreadPoolExecutor(max_workers=1))
    TerminalTeams_pb2_grpc.add_NeighborsDiscoveryServicer_to_server(Servicer(), server)
    server.add_insecure_port(src_ip_address + ":" + src_port)
    server.start()
    logging.info("Server started, listening on " + src_ip_address + ":" + src_port)
    server_shutdown_event.wait()
    server.stop(3).wait()

def scenario_4_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 4.1...")
    client_shutdown_event = threading.Event()
    greet_counter = 0
    greet_limit = 5
    def stop_client(signum, frame):
        server_shutdown_event.set()
    def count_greet():
        greet_counter += 1
        if greet_counter >= greet_limit:
            stop_client(0, 0)
    signal.signal(signal.SIGTERM, stop_client)
    with grpc.insecure_channel(dst_ip_address + ":" + dst_port) as channel:
        stub = TerminalTeams_pb2_grpc.NeighborsDiscoveryStub(channel)
        while not client_shutdown_event.is_set():
            try:
                logging.info("Sending dummy GreetRequest...")
                stub.Heartbeat(TerminalTeams_pb2.GreetRequest(nickname="nickname", identity="identity", ipAddressAndPort=src_ip_address + ":" + src_port))
            except Exception:
                logging.error("Failed to send dummy GreetRequest!")
                pass
            count_greet()
            time.sleep(2)
    client_shutdown_event.wait()

def scenario_5_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 5.1...")
    server_shutdown_event = threading.Event()
    def stop_server(signum, frame):
        server_shutdown_event.set()
    class Servicer(TerminalTeams_pb2_grpc.NeighborsChat):
        def Tell(self, request, context):
            logging.info("Sending dummy TellReply...")
            return TerminalTeams_pb2.TellReply(identity="identity")
    signal.signal(signal.SIGTERM, stop_server)
    server = grpc.server(concurrent.futures.ThreadPoolExecutor(max_workers=1))
    TerminalTeams_pb2_grpc.add_NeighborsDiscoveryServicer_to_server(Servicer(), server)
    server.add_insecure_port(src_ip_address + ":" + src_port)
    server.start()
    logging.info("Server started, listening on " + src_ip_address + ":" + src_port)
    server_shutdown_event.wait()
    server.stop(3).wait()

def scenario_5_2(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 5.2...")
    server_shutdown_event = threading.Event()
    def stop_server(signum, frame):
        server_shutdown_event.set()
    sequence_numbers = {}
    class Servicer(TerminalTeams_pb2_grpc.NeighborsChat):
        def Tell(self, request, context):
            logging.info("Sending dummy TellReply...")
            if request.sequenceNumber in sequence_numbers:
                return TerminalTeams_pb2.TellReply(identity="identity")
            sequence_numbers.add(request.sequenceNumber)
            return TerminalTeams_pb2.TellReply(identity="error")
    signal.signal(signal.SIGTERM, stop_server)
    server = grpc.server(concurrent.futures.ThreadPoolExecutor(max_workers=1))
    TerminalTeams_pb2_grpc.add_NeighborsDiscoveryServicer_to_server(Servicer(), server)
    server.add_insecure_port(src_ip_address + ":" + src_port)
    server.start()
    logging.info("Server started, listening on " + src_ip_address + ":" + src_port)
    server_shutdown_event.wait()
    server.stop(3).wait()

def scenario_6_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 6.1...")
    client_shutdown_event = threading.Event()
    tell_counter = 0
    tell_limit = 5
    def stop_client(signum, frame):
        server_shutdown_event.set()
    def count_tell():
        tell_counter += 1
        if tell_counter >= tell_limit:
            stop_client(0, 0)
    signal.signal(signal.SIGTERM, stop_client)
    with grpc.insecure_channel(dst_ip_address + ":" + dst_port) as channel:
        stub = TerminalTeams_pb2_grpc.NeighborsChatStub(channel)
        while not client_shutdown_event.is_set():
            try:
                logging.info("Sending dummy TellRequest...")
                stub.Tell(TerminalTeams_pb2.TellRequest(identity="identity", message=("msg" + str(tell_counter)), sequenceNumber=tell_counter, timestamp=""))
            except Exception:
                logging.error("Failed to send dummy TellRequest!")
                pass
            count_tell()
            time.sleep(2)
    client_shutdown_event.wait()

def scenario_7_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 7.1...")
    server_shutdown_event = threading.Event()
    def stop_server(signum, frame):
        server_shutdown_event.set()
    class Servicer(TerminalTeams_pb2_grpc.NeighborsChat):
        def Narrate(self, request_iterator, context):
            logging.info("Sending dummy NarrateReply...")
            return TerminalTeams_pb2.NarrateReply(identity="identity")
    signal.signal(signal.SIGTERM, stop_server)
    server = grpc.server(concurrent.futures.ThreadPoolExecutor(max_workers=1))
    TerminalTeams_pb2_grpc.add_NeighborsDiscoveryServicer_to_server(Servicer(), server)
    server.add_insecure_port(src_ip_address + ":" + src_port)
    server.start()
    logging.info("Server started, listening on " + src_ip_address + ":" + src_port)
    server_shutdown_event.wait()
    server.stop(3).wait()

def scenario_7_2(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 7.2...")
    server_shutdown_event = threading.Event()
    def stop_server(signum, frame):
        server_shutdown_event.set()
    sequence_numbers = {}
    class Servicer(TerminalTeams_pb2_grpc.NeighborsChat):
        def Narrate(self, request_iterator, context):
            logging.info("Sending dummy NarrateReply...")
            if request.sequenceNumber in sequence_numbers:
                return TerminalTeams_pb2.NarrateReply(identity="identity")
            sequence_numbers.add(request.sequenceNumber)
            return TerminalTeams_pb2.NarrateReply(identity="error")
    signal.signal(signal.SIGTERM, stop_server)
    server = grpc.server(concurrent.futures.ThreadPoolExecutor(max_workers=1))
    TerminalTeams_pb2_grpc.add_NeighborsDiscoveryServicer_to_server(Servicer(), server)
    server.add_insecure_port(src_ip_address + ":" + src_port)
    server.start()
    logging.info("Server started, listening on " + src_ip_address + ":" + src_port)
    server_shutdown_event.wait()
    server.stop(3).wait()

# def scenario_8_1(src_ip_address, src_port, dst_ip_address, dst_port):
#     logging.info("Running scenario 8.1...")
#     client_shutdown_event = threading.Event()
#     def stop_client(signum, frame):
#         server_shutdown_event.set()
#     signal.signal(signal.SIGTERM, stop_client)
#     with grpc.insecure_channel(dst_ip_address + ":" + dst_port) as channel:
#         stub = TerminalTeams_pb2_grpc.NeighborsChatStub(channel)
#         try:
#             logging.info("Sending dummy NarrateRequest...")
#             stub.Narrate(TerminalTeams_pb2.NarrateRequest(identity="identity", message=("msg" + str(narrate_counter)), sequenceNumber=tell_counter, timestamp=""))
#         except Exception:
#             logging.error("Failed to send dummy NarrateRequest!")
#             pass
#         time.sleep(2)
#     client_shutdown_event.wait()

scenarios = {
    '1.1': scenario_1_1,
    '1.2': scenario_1_2,
    '2.1': scenario_2_1,
    '3.1': scenario_3_1,
    '4.1': scenario_4_1,
    '5.1': scenario_5_1,
    '5.2': scenario_5_2,
    '6.1': scenario_6_1,
    '7.1': scenario_7_1,
    '7.2': scenario_7_2,
    '8.1': scenario_8_1
}

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO, format='%(asctime)s %(levelname)s: %(message)s')
    parser = argparse.ArgumentParser()
    parser.add_argument("--src-ip-address", dest="src_ip_address", help="Source (host) IP address", required=True)
    parser.add_argument("--dst-ip-address", dest="dst_ip_address", help="Destination (client) IP address", required=True)
    parser.add_argument("--src-port", dest="src_port", help="Source (host) port", required=True)
    parser.add_argument("--dst-port", dest="dst_port", help="Destination (client) port", required=True)
    parser.add_argument("--scenario", dest="scenario", help="Scenario (behavior) of the dummy host", required=True)
    args = parser.parse_args()
    if not args.scenario in scenarios:
        logger.error("Non-existing scenario " + args.scenario + " was provided!")
    scenarios[args.scenario](args.src_ip_address, args.src_port, args.dst_ip_address, args.dst_port)
