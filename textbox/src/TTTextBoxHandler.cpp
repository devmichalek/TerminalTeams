#include "TTTextBoxHandler.hpp"
#include "TTTextBoxMessage.hpp"
#include "TTDiagnosticsLogger.hpp"

TTTextBoxHandler::TTTextBoxHandler(const TTTextBoxSettings& settings,
    TTTextBoxCallbackMessageSent callbackMessageSent,
    TTTextBoxCallbackContactSwitch callbackContactsSwitch) :
        mPipe(settings.getNamedPipe()),
        mCallbackMessageSent(callbackMessageSent),
        mCallbackContactsSwitch(callbackContactsSwitch),
        mStopped{false} {
    LOG_INFO("Constructing...");
    // Create pipe
    if (!mPipe->create()) {
        throw std::runtime_error("TTTextBoxHandler: Failed to create named pipe!");
    }
    // Set main receiver thread
    std::promise<void> mainPromise;
    mBlockers.push_back(mainPromise.get_future());
    mThreads.push_back(std::thread(&TTTextBoxHandler::main, this, std::move(mainPromise)));
    mThreads.back().detach();
    LOG_INFO("Successfully constructed!");
}

TTTextBoxHandler::~TTTextBoxHandler() {
    LOG_INFO("Destructing...");
    stop();
    for (auto &blocker : mBlockers) {
        blocker.wait();
    }
    LOG_INFO("Successfully destructed!");
}

void TTTextBoxHandler::stop() {
    LOG_WARNING("Forced stop...");
    mStopped.store(true);
}

bool TTTextBoxHandler::stopped() const {
    return mStopped.load();
}

void TTTextBoxHandler::main(std::promise<void> promise) {
    LOG_INFO("Started textbox handler loop");
    if (!mPipe->alive()) {
        LOG_ERROR("Failed to run, pipe is not alive!");
    } else {
        try {
            for (auto i = TTTextBoxHandler::RECEIVE_TRY_COUNT; i > 0; --i) {
                if (stopped()) {
                    break;
                }
                TTTextBoxMessage message(TTTextBoxStatus::UNDEFINED, 0, nullptr);
                if (!mPipe->receive(reinterpret_cast<char*>(&message))) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(TTTextBoxHandler::RECEIVE_TIMEOUT_MS));
                    continue;
                }
                switch (message.status) {
                    case TTTextBoxStatus::HEARTBEAT:
                        LOG_INFO("Received heartbeat message");
                        break;
                    case TTTextBoxStatus::CONTACTS_SWITCH:
                    {
                        LOG_INFO("Received contacts switch message");
                        size_t id = 0;
                        memcpy(&id, message.data, message.dataLength);
                        mCallbackContactsSwitch(id);
                        break;
                    }
                    case TTTextBoxStatus::MESSAGE:
                    {
                        LOG_INFO("Received message");
                        mCallbackMessageSent({message.data, message.dataLength});
                        break;
                    }
                    case TTTextBoxStatus::GOODBYE:
                        LOG_WARNING("Received goodbye message");
                        throw std::runtime_error({});
                    case TTTextBoxStatus::UNDEFINED:
                    default:
                        LOG_ERROR("Received undefined message!");
                        throw std::runtime_error({});
                }
                i = TTTextBoxHandler::RECEIVE_TRY_COUNT + 1;
            }
        } catch (...) {
            LOG_ERROR("Caught unknown exception!");
        }
    }
    stop();
    promise.set_value();
    LOG_INFO("Completed textbox handler loop");
}
