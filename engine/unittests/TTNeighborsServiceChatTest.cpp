#include "TTNeighborsServiceChat.hpp"
#include "TTBroadcasterChatMock.hpp"
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <deque>

using ::testing::_;
using ::testing::Return;

TEST(TTNeighborsServiceChatTest, HappyPathTell) {
    const std::string identity1 = "identity1";
    const std::string identity2 = "identity2";
    const std::string message1 = "msg";
    grpc::ServerContext context;
    tt::TellRequest request;
    request.set_identity(identity1);
    request.set_message(message1);
    const TTTellRequest expectedRequest(identity1, message1);
    tt::TellReply reply;
    TTBroadcasterChatMock handler;
    EXPECT_CALL(handler, handleReceive(expectedRequest))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(handler, getIdentity())
        .Times(1)
        .WillOnce(Return(identity2));
    TTNeighborsServiceChat service(handler);
    EXPECT_TRUE(service.Tell(&context, &request, &reply).ok());
    EXPECT_EQ(reply.identity(), identity2);
}

TEST(TTNeighborsServiceChatTest, UnhappyPathTellContextIsNull) {
    tt::TellRequest request;
    tt::TellReply reply;
    TTBroadcasterChatMock handler;
    TTNeighborsServiceChat service(handler);
    EXPECT_FALSE(service.Tell(nullptr, &request, &reply).ok());
}

TEST(TTNeighborsServiceChatTest, UnhappyPathTellRequestIsNull) {
    grpc::ServerContext context;
    tt::TellReply reply;
    TTBroadcasterChatMock handler;
    TTNeighborsServiceChat service(handler);
    EXPECT_FALSE(service.Tell(&context, nullptr, &reply).ok());
}

TEST(TTNeighborsServiceChatTest, UnhappyPathTellReplyIsNull) {
    grpc::ServerContext context;
    tt::TellRequest request;
    TTBroadcasterChatMock handler;
    TTNeighborsServiceChat service(handler);
    EXPECT_FALSE(service.Tell(&context, &request, nullptr).ok());
}

TEST(TTNeighborsServiceChatTest, UnhappyPathTellHandlerFailed) {
    const std::string identity1 = "identity1";
    const std::string message1 = "msg";
    grpc::ServerContext context;
    tt::TellRequest request;
    request.set_identity(identity1);
    request.set_message(message1);
    const TTTellRequest expectedRequest(identity1, message1);
    tt::TellReply reply;
    TTBroadcasterChatMock handler;
    EXPECT_CALL(handler, handleReceive(expectedRequest))
        .Times(1)
        .WillOnce(Return(false));
    TTNeighborsServiceChat service(handler);
    EXPECT_FALSE(service.Tell(&context, &request, &reply).ok());
}

class ServerReaderInterfaceNarrateRequest : public ::grpc::ServerReaderInterface<::tt::NarrateRequest> {
public:
    MOCK_METHOD(bool, NextMessageSize, (uint32_t* sz), (override));
    MOCK_METHOD(bool, Read, (tt::NarrateRequest* msg), (override));
    MOCK_METHOD(void, SendInitialMetadata, (), (override));
};

TEST(TTNeighborsServiceChatTest, HappyPathNarrate) {
    const std::string identity1 = "identity1";
    const std::string identity2 = "identity2";
    const std::string message1 = "msg1";
    const std::string message2 = "msg2";
    grpc::ServerContext context;
    size_t counter = 0;
    auto srinr = std::make_unique<ServerReaderInterfaceNarrateRequest>();
    auto srnr = reinterpret_cast<grpc::ServerReader<tt::NarrateRequest>*>(srinr.get());
    EXPECT_CALL(*srinr, Read(_))
        .Times(3)
        .WillRepeatedly([&](tt::NarrateRequest* msg) {
            const auto value = counter++;
            if (value == 0) {
                msg->set_identity(identity1);
                msg->set_message(message1);
                return true;
            } else if (value == 1) {
                msg->set_identity(identity1);
                msg->set_message(message2);
                return true;
            } else {
                return false;
            }
        });
    const TTNarrateRequest expectedRequest(identity1, {message1, message2});
    tt::NarrateReply reply;
    TTBroadcasterChatMock handler;
    EXPECT_CALL(handler, handleReceive(expectedRequest))
        .Times(1)
        .WillOnce(Return(true));
    EXPECT_CALL(handler, getIdentity())
        .Times(1)
        .WillOnce(Return(identity2));
    TTNeighborsServiceChat service(handler);
    EXPECT_TRUE(service.Narrate(&context, srnr, &reply).ok());
    EXPECT_EQ(reply.identity(), identity2);
}

TEST(TTNeighborsServiceChatTest, UnhappyPathNarrateContextIsNull) {
    auto srinr = std::make_unique<ServerReaderInterfaceNarrateRequest>();
    auto srnr = reinterpret_cast<grpc::ServerReader<tt::NarrateRequest>*>(srinr.get());
    tt::NarrateReply reply;
    TTBroadcasterChatMock handler;
    TTNeighborsServiceChat service(handler);
    EXPECT_FALSE(service.Narrate(nullptr, srnr, &reply).ok());
}

TEST(TTNeighborsServiceChatTest, UnhappyPathNarrateStreamIsNull) {
    grpc::ServerContext context;
    tt::NarrateReply reply;
    TTBroadcasterChatMock handler;
    TTNeighborsServiceChat service(handler);
    EXPECT_FALSE(service.Narrate(&context, nullptr, &reply).ok());
}

TEST(TTNeighborsServiceChatTest, UnhappyPathNarrateReplyIsNull) {
    grpc::ServerContext context;
    auto srinr = std::make_unique<ServerReaderInterfaceNarrateRequest>();
    auto srnr = reinterpret_cast<grpc::ServerReader<tt::NarrateRequest>*>(srinr.get());
    TTBroadcasterChatMock handler;
    TTNeighborsServiceChat service(handler);
    EXPECT_FALSE(service.Narrate(&context, srnr, nullptr).ok());
}

TEST(TTNeighborsServiceChatTest, UnhappyPathNarrateHandlerFailed) {
    const std::string identity1 = "identity1";
    const std::string identity2 = "identity2";
    const std::string message1 = "msg1";
    const std::string message2 = "msg2";
    grpc::ServerContext context;
    size_t counter = 0;
    auto srinr = std::make_unique<ServerReaderInterfaceNarrateRequest>();
    auto srnr = reinterpret_cast<grpc::ServerReader<tt::NarrateRequest>*>(srinr.get());
    EXPECT_CALL(*srinr, Read(_))
        .Times(3)
        .WillRepeatedly([&](tt::NarrateRequest* msg) {
            const auto value = counter++;
            if (value == 0) {
                msg->set_identity(identity1);
                msg->set_message(message1);
                return true;
            } else if (value == 1) {
                msg->set_identity(identity1);
                msg->set_message(message2);
                return true;
            } else {
                return false;
            }
        });
    const TTNarrateRequest expectedRequest(identity1, {message1, message2});
    tt::NarrateReply reply;
    TTBroadcasterChatMock handler;
    EXPECT_CALL(handler, handleReceive(expectedRequest))
        .Times(1)
        .WillOnce(Return(false));
    TTNeighborsServiceChat service(handler);
    EXPECT_FALSE(service.Narrate(&context, srnr, &reply).ok());
}

TEST(TTNeighborsServiceChatTest, UnhappyPathNarrateManyUniqueIds) {
    const std::string identity1 = "identity1";
    const std::string identity2 = "identity2";
    const std::string message1 = "msg1";
    const std::string message2 = "msg2";
    grpc::ServerContext context;
    size_t counter = 0;
    auto srinr = std::make_unique<ServerReaderInterfaceNarrateRequest>();
    auto srnr = reinterpret_cast<grpc::ServerReader<tt::NarrateRequest>*>(srinr.get());
    EXPECT_CALL(*srinr, Read(_))
        .Times(3)
        .WillRepeatedly([&](tt::NarrateRequest* msg) {
            const auto value = counter++;
            if (value == 0) {
                msg->set_identity(identity1);
                msg->set_message(message1);
                return true;
            } else if (value == 1) {
                msg->set_identity(identity2);
                msg->set_message(message2);
                return true;
            } else {
                return false;
            }
        });
    const TTNarrateRequest expectedRequest(identity1, {message1, message2});
    tt::NarrateReply reply;
    TTBroadcasterChatMock handler;
    TTNeighborsServiceChat service(handler);
    EXPECT_FALSE(service.Narrate(&context, srnr, &reply).ok());
}
