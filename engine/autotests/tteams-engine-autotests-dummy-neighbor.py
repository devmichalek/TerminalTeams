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
# 1.3 Never reply with HeartbeatReply on HeartbeatRequest
# 2.1 Request with HeartbeatRequest every t seconds
# 2.2 Never request with HeartbeatRequest
# 3.1 Always reply with GreetReply on GreetRequest
# 3.2 Never reply with GreetReply on GreetRequest
# 4.1 Request with GreetRequest n times every t seconds until first GreetReply
# 4.2 Never request with GreetRequest
# 5.1 Always reply with TellReply on TellRequest
# 5.2 Reply with TellReply every n times for particular TellRequest sequence number
# 5.3 Never reply with TellReply on TellRequest
# 6.1 Request with TellRequest every n received message
# 6.2 Request with TellRequest every t seconds, up to n messages
# 6.3 Never request with TellRequest
# 7.1 Always reply with NarrateReply on NarrateReply
# 7.2 Reply with NarratReply every second time on NarrateReply
# 7.3 Never reply with NarrateReply on NarrateReply
# 8.1 Request with NarrateRequest once
# 8.2 Never request with NarrateRequest

import argparse
import logging
import grpc
import concurrent
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
            return TerminalTeams_pb2.HeartbeatReply(identity="dummy")
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
            return TerminalTeams_pb2.HeartbeatReply(identity="dummy")
    signal.signal(signal.SIGTERM, stop_server)
    server = grpc.server(futures.ThreadPoolExecutor(max_workers=1))
    TerminalTeams_pb2_grpc.add_NeighborsDiscoveryServicer_to_server(Servicer(), server)
    server.add_insecure_port(src_ip_address + ":" + src_port)
    server.start()
    logging.info("Server started, listening on " + src_ip_address + ":" + src_port)
    server_shutdown_event.wait()
    server.stop(3).wait()

def scenario_1_3(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 1.3...")

def scenario_2_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 2.1...")

def scenario_2_2(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 2.2...")

def scenario_3_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 3.1...")

def scenario_3_2(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 3.2...")

def scenario_4_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 4.1...")

def scenario_4_2(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 4.2...")

def scenario_5_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 5.1...")

def scenario_5_2(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 5.2...")

def scenario_5_3(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 5.3...")

def scenario_6_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 6.1...")

def scenario_6_2(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 6.2...")

def scenario_6_3(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 6.3...")

def scenario_7_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 7.1...")

def scenario_7_2(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 7.2...")

def scenario_7_3(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 7.3...")

def scenario_8_1(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 8.1...")

def scenario_8_2(src_ip_address, src_port, dst_ip_address, dst_port):
    logging.info("Running scenario 8.2...")

scenarios = {
    '1.1': scenario_1_1,
    '1.2': scenario_1_2,
    '1.3': scenario_1_3,
    '2.1': scenario_2_1,
    '2.2': scenario_2_2,
    '3.1': scenario_3_1,
    '3.2': scenario_3_2,
    '4.1': scenario_4_1,
    '4.2': scenario_4_2,
    '5.1': scenario_5_1,
    '5.2': scenario_5_2,
    '5.3': scenario_5_3,
    '6.1': scenario_6_1,
    '6.2': scenario_6_2,
    '6.3': scenario_6_3,
    '7.1': scenario_7_1,
    '7.2': scenario_7_2,
    '7.3': scenario_7_3,
    '8.1': scenario_8_1,
    '8.2': scenario_8_2
}

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO)
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
