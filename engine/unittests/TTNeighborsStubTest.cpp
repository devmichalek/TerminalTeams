#include "TTNeighborsStub.hpp"
#include "TTNeighborsChatStubMock.hpp"
#include "TTNeighborsDiscoveryStubMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using ::testing::_;
using ::testing::Return;

TEST(TTNeighborsStubTest, HappyPathSendTell) {
    const TTTellRequest request("5ef885a", "Hello world!");
    TTNeighborsChatStubMock chatStub({});
    EXPECT_CALL(chatStub, Tell(_, _, _))
        .Times(1)
        .WillOnce([&](::grpc::ClientContext* context, const ::tt::TellRequest& request, ::tt::TellReply* response){
            if (context) {
                response->set_identity(request.identity());
                return grpc::Status();
            }
            return grpc::Status(grpc::INVALID_ARGUMENT, "Context is null!");
        });
    TTNeighborsStub stub;
    const auto response = stub.sendTell(chatStub, request);
    EXPECT_TRUE(response.status);
}

TEST(TTNeighborsStubTest, UnhappyPathSendTell) {
    const TTTellRequest request("5ef885a", "Hello world!");
    TTNeighborsChatStubMock chatStub({});
    EXPECT_CALL(chatStub, Tell(_, _, _))
        .Times(1)
        .WillOnce([&](::grpc::ClientContext* context, const ::tt::TellRequest& request, ::tt::TellReply* response){
            return grpc::Status(grpc::UNKNOWN, "?");
        });
    TTNeighborsStub stub;
    const auto response = stub.sendTell(chatStub, request);
    EXPECT_FALSE(response.status);
}

class ClientWriterInterfaceNarrateRequest : public ::grpc::ClientWriterInterface<::tt::NarrateRequest> {
public:
    MOCK_METHOD(bool, Write, (const ::tt::NarrateRequest& msg, grpc::WriteOptions options), (override));
    MOCK_METHOD(grpc::Status, Finish, (), (override));
    MOCK_METHOD(bool, WritesDone, (), (override));
};

MATCHER_P(IsNarrateRequestEqualTo, n, "") {
    bool result = (arg.identity() == n.identity());
    result &= (arg.message() == n.message());
    return result;
}

TEST(TTNeighborsStubTest, HappyPathSendNarrate) {
    const std::string identity = "5ef885a";
    const std::string message1 = "Hello ";
    const std::string message2 = "world!";
    tt::NarrateRequest request1;
    request1.set_identity(identity);
    request1.set_message(message1);
    tt::NarrateRequest request2;
    request2.set_identity(identity);
    request2.set_message(message2);
    const TTNarrateRequest request(identity, {message1, message2});
    auto cwinr = std::make_unique<ClientWriterInterfaceNarrateRequest>();
    EXPECT_CALL(*cwinr, Write(IsNarrateRequestEqualTo(request1), _))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*cwinr, Write(IsNarrateRequestEqualTo(request2), _))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*cwinr, WritesDone())
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*cwinr, Finish())
        .Times(1)
        .WillOnce(Return(grpc::Status()));
    TTNeighborsChatStubMock chatStub({});
    EXPECT_CALL(chatStub, NarrateRaw(_, _))
        .Times(1)
        .WillOnce([&](::grpc::ClientContext* context, ::tt::NarrateReply* response){
            ClientWriterInterfaceNarrateRequest* result = nullptr;
            if (context) {
                result = cwinr.release();
            }
            return result;
        });
    TTNeighborsStub stub;
    const auto response = stub.sendNarrate(chatStub, request);
    EXPECT_TRUE(response.status);
}

TEST(TTNeighborsStubTest, UnhappyPathSendNarrateFailedToCreateWriter) {
    const TTNarrateRequest request;
    TTNeighborsChatStubMock chatStub({});
    EXPECT_CALL(chatStub, NarrateRaw(_, _))
        .Times(1)
        .WillOnce([&](::grpc::ClientContext* context, ::tt::NarrateReply* response){
            ClientWriterInterfaceNarrateRequest* result = nullptr;
            return result;
        });
    TTNeighborsStub stub;
    const auto response = stub.sendNarrate(chatStub, request);
    EXPECT_FALSE(response.status);
}

TEST(TTNeighborsStubTest, UnhappyPathSendNarrateFailedToWrite) {
    const std::string identity = "5ef885a";
    const std::string message1 = "Hello ";
    const std::string message2 = "world!";
    tt::NarrateRequest request1;
    request1.set_identity(identity);
    request1.set_message(message1);
    tt::NarrateRequest request2;
    request2.set_identity(identity);
    request2.set_message(message2);
    const TTNarrateRequest request(identity, {message1, message2});
    auto cwinr = std::make_unique<ClientWriterInterfaceNarrateRequest>();
    EXPECT_CALL(*cwinr, Write(IsNarrateRequestEqualTo(request1), _))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*cwinr, Write(IsNarrateRequestEqualTo(request2), _))
        .Times(1)
        .WillOnce(Return(false));
    TTNeighborsChatStubMock chatStub({});
    EXPECT_CALL(chatStub, NarrateRaw(_, _))
        .Times(1)
        .WillOnce([&](::grpc::ClientContext* context, ::tt::NarrateReply* response){
            ClientWriterInterfaceNarrateRequest* result = nullptr;
            if (context) {
                result = cwinr.release();
            }
            return result;
        });
    TTNeighborsStub stub;
    const auto response = stub.sendNarrate(chatStub, request);
    EXPECT_FALSE(response.status);
}

TEST(TTNeighborsStubTest, UnhappyPathSendNarrateFailedToFinish) {
    const std::string identity = "5ef885a";
    const std::string message1 = "Hello ";
    const std::string message2 = "world!";
    tt::NarrateRequest request1;
    request1.set_identity(identity);
    request1.set_message(message1);
    tt::NarrateRequest request2;
    request2.set_identity(identity);
    request2.set_message(message2);
    const TTNarrateRequest request(identity, {message1, message2});
    auto cwinr = std::make_unique<ClientWriterInterfaceNarrateRequest>();
    EXPECT_CALL(*cwinr, Write(IsNarrateRequestEqualTo(request1), _))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*cwinr, Write(IsNarrateRequestEqualTo(request2), _))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*cwinr, WritesDone())
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(*cwinr, Finish())
        .Times(1)
        .WillOnce(Return(grpc::Status(grpc::UNKNOWN, "?")));
    TTNeighborsChatStubMock chatStub({});
    EXPECT_CALL(chatStub, NarrateRaw(_, _))
        .Times(1)
        .WillOnce([&](::grpc::ClientContext* context, ::tt::NarrateReply* response){
            ClientWriterInterfaceNarrateRequest* result = nullptr;
            if (context) {
                result = cwinr.release();
            }
            return result;
        });
    TTNeighborsStub stub;
    const auto response = stub.sendNarrate(chatStub, request);
    EXPECT_FALSE(response.status);
}

TEST(TTNeighborsStubTest, HappyPathSendGreet) {
    const TTGreetRequest request("adriqun", "5ef885a", "192.168.1.8:88");
    TTNeighborsDiscoveryStubMock discoveryStub({});
    EXPECT_CALL(discoveryStub, Greet(_, _, _))
        .Times(1)
        .WillOnce([&](::grpc::ClientContext* context, const ::tt::GreetRequest& request, ::tt::GreetReply* response){
            if (context) {
                response->set_nickname(request.nickname());
                response->set_identity(request.identity());
                response->set_ipaddressandport(request.ipaddressandport());
                return grpc::Status();
            }
            return grpc::Status(grpc::INVALID_ARGUMENT, "Context is null!");
        });
    TTNeighborsStub stub;
    const auto response = stub.sendGreet(discoveryStub, request);
    EXPECT_TRUE(response.status);
    EXPECT_EQ(response.nickname, request.nickname);
    EXPECT_EQ(response.identity, request.identity);
    EXPECT_EQ(response.ipAddressAndPort, request.ipAddressAndPort);
}

TEST(TTNeighborsStubTest, UnhappyPathSendGreet) {
    const TTGreetRequest request("adriqun", "5ef885a", "192.168.1.8:88");
    TTNeighborsDiscoveryStubMock discoveryStub({});
    EXPECT_CALL(discoveryStub, Greet(_, _, _))
        .Times(1)
        .WillOnce([&](::grpc::ClientContext* context, const ::tt::GreetRequest& request, ::tt::GreetReply* response){
            return grpc::Status(grpc::UNKNOWN, "?");
        });
    TTNeighborsStub stub;
    const auto response = stub.sendGreet(discoveryStub, request);
    EXPECT_FALSE(response.status);
}

TEST(TTNeighborsStubTest, HappyPathSendHeartbeat) {
    const TTHeartbeatRequest request("5ef885a");
    TTNeighborsDiscoveryStubMock discoveryStub({});
    EXPECT_CALL(discoveryStub, Heartbeat(_, _, _))
        .Times(1)
        .WillOnce([&](::grpc::ClientContext* context, const ::tt::HeartbeatRequest& request, ::tt::HeartbeatReply* response){
            if (context) {
                response->set_identity(request.identity());
                return grpc::Status();
            }
            return grpc::Status(grpc::INVALID_ARGUMENT, "Context is null!");
        });
    TTNeighborsStub stub;
    const auto response = stub.sendHeartbeat(discoveryStub, request);
    EXPECT_TRUE(response.status);
    EXPECT_EQ(response.identity, request.identity);
}

TEST(TTNeighborsStubTest, UnhappyPathSendHeartbeat) {
    const TTHeartbeatRequest request("5ef885a");
    TTNeighborsDiscoveryStubMock discoveryStub({});
    EXPECT_CALL(discoveryStub, Heartbeat(_, _, _))
        .Times(1)
        .WillOnce([&](::grpc::ClientContext* context, const ::tt::HeartbeatRequest& request, ::tt::HeartbeatReply* response){
            return grpc::Status(grpc::UNKNOWN, "?");
        });
    TTNeighborsStub stub;
    const auto response = stub.sendHeartbeat(discoveryStub, request);
    EXPECT_FALSE(response.status);
}
