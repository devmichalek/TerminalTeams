#include "TTNeighborsServiceDiscovery.hpp"
#include "TTBroadcasterDiscoveryMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Return;

TEST(TTNeighborsServiceDiscoveryTest, HappyPathGreet) {
    const std::string nickname1 = "nickname1";
    const std::string nickname2 = "nickname2";
    const std::string identity1 = "identity1";
    const std::string identity2 = "identity2";
    const std::string ipAddressAndPort1 = "192.168.1.17:17";
    const std::string ipAddressAndPort2 = "192.168.1.18:18";
    grpc::ServerContext context;
    tt::GreetRequest request;
    request.set_nickname(nickname1);
    request.set_identity(identity1);
    request.set_ipaddressandport(ipAddressAndPort1);
    TTGreetRequest expectedRequest(nickname1, identity1, ipAddressAndPort1);
    tt::GreetReply reply;
    TTBroadcasterDiscoveryMock handler;
    EXPECT_CALL(handler, handleGreet(expectedRequest))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(handler, getNickname())
        .Times(1)
        .WillOnce(Return(nickname2));
    EXPECT_CALL(handler, getIdentity())
        .Times(1)
        .WillOnce(Return(identity2));
    EXPECT_CALL(handler, getIpAddressAndPort())
        .Times(1)
        .WillOnce(Return(ipAddressAndPort2));
    TTNeighborsServiceDiscovery service(handler);
    EXPECT_TRUE(service.Greet(&context, &request, &reply).ok());
    EXPECT_EQ(reply.nickname(), nickname2);
    EXPECT_EQ(reply.identity(), identity2);
    EXPECT_EQ(reply.ipaddressandport(), ipAddressAndPort2);
}

TEST(TTNeighborsServiceDiscoveryTest, UnhappyPathGreetContextIsNull) {
    tt::GreetRequest request;
    tt::GreetReply reply;
    TTBroadcasterDiscoveryMock handler;
    TTNeighborsServiceDiscovery service(handler);
    EXPECT_FALSE(service.Greet(nullptr, &request, &reply).ok());
}

TEST(TTNeighborsServiceDiscoveryTest, UnhappyPathGreetRequestIsNull) {
    grpc::ServerContext context;
    tt::GreetReply reply;
    TTBroadcasterDiscoveryMock handler;
    TTNeighborsServiceDiscovery service(handler);
    EXPECT_FALSE(service.Greet(&context, nullptr, &reply).ok());
}

TEST(TTNeighborsServiceDiscoveryTest, UnhappyPathGreetReplyIsNull) {
    grpc::ServerContext context;
    tt::GreetRequest request;
    TTBroadcasterDiscoveryMock handler;
    TTNeighborsServiceDiscovery service(handler);
    EXPECT_FALSE(service.Greet(&context, &request, nullptr).ok());
}

TEST(TTNeighborsServiceDiscoveryTest, UnhappyPathGreetHandlerFailed) {
    const std::string nickname1 = "nickname1";
    const std::string identity1 = "identity1";
    const std::string ipAddressAndPort1 = "192.168.1.17:17";
    grpc::ServerContext context;
    tt::GreetRequest request;
    request.set_nickname(nickname1);
    request.set_identity(identity1);
    request.set_ipaddressandport(ipAddressAndPort1);
    TTGreetRequest expectedRequest(nickname1, identity1, ipAddressAndPort1);
    tt::GreetReply reply;
    TTBroadcasterDiscoveryMock handler;
    EXPECT_CALL(handler, handleGreet(expectedRequest))
        .Times(1)
        .WillOnce(Return(false));
    TTNeighborsServiceDiscovery service(handler);
    EXPECT_FALSE(service.Greet(&context, &request, &reply).ok());
}

TEST(TTNeighborsServiceDiscoveryTest, HappyPathHeartbeat) {
    const std::string identity1 = "identity1";
    const std::string identity2 = "identity2";
    grpc::ServerContext context;
    tt::HeartbeatRequest request;
    request.set_identity(identity1);
    TTHeartbeatRequest expectedRequest(identity1);
    tt::HeartbeatReply reply;
    TTBroadcasterDiscoveryMock handler;
    EXPECT_CALL(handler, handleHeartbeat(expectedRequest))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(handler, getIdentity())
        .Times(1)
        .WillOnce(Return(identity2));
    TTNeighborsServiceDiscovery service(handler);
    EXPECT_TRUE(service.Heartbeat(&context, &request, &reply).ok());
    EXPECT_EQ(reply.identity(), identity2);
}

TEST(TTNeighborsServiceDiscoveryTest, UnhappyPathHeartbeatContextIsNull) {
    tt::HeartbeatRequest request;
    tt::HeartbeatReply reply;
    TTBroadcasterDiscoveryMock handler;
    TTNeighborsServiceDiscovery service(handler);
    EXPECT_FALSE(service.Heartbeat(nullptr, &request, &reply).ok());
}

TEST(TTNeighborsServiceDiscoveryTest, UnhappyPathHeartbeatRequestIsNull) {
    grpc::ServerContext context;
    tt::HeartbeatReply reply;
    TTBroadcasterDiscoveryMock handler;
    TTNeighborsServiceDiscovery service(handler);
    EXPECT_FALSE(service.Heartbeat(&context, nullptr, &reply).ok());
}

TEST(TTNeighborsServiceDiscoveryTest, UnhappyPathHeartbeatReplyIsNull) {
    grpc::ServerContext context;
    tt::HeartbeatRequest request;
    TTBroadcasterDiscoveryMock handler;
    TTNeighborsServiceDiscovery service(handler);
    EXPECT_FALSE(service.Heartbeat(&context, &request, nullptr).ok());
}

TEST(TTNeighborsServiceDiscoveryTest, UnhappyPathHeartbeatHandlerFailed) {
    const std::string identity1 = "identity1";
    grpc::ServerContext context;
    tt::HeartbeatRequest request;
    request.set_identity(identity1);
    TTHeartbeatRequest expectedRequest(identity1);
    tt::HeartbeatReply reply;
    TTBroadcasterDiscoveryMock handler;
    EXPECT_CALL(handler, handleHeartbeat(expectedRequest))
        .Times(1)
        .WillOnce(Return(false));
    TTNeighborsServiceDiscovery service(handler);
    EXPECT_FALSE(service.Heartbeat(&context, &request, &reply).ok());
}
