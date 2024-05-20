#include "TTTextBoxHandler.hpp"
#include "TTTextBoxSettings.hpp"
#include "TTTextBoxMessage.hpp"

TTTextBoxHandler::TTTextBoxHandler(const TTTextBoxSettings& settings,
    TTTextBoxCallbackMessageSent callbackMessageSent,
    TTTextBoxCallbackContactSwitch callbackContactsSwitch) :
        mPipe(settings.getNamedPipe()),
        mCallbackMessageSent(callbackMessageSent),
        mCallbackContactsSwitch(callbackContactsSwitch),
        mStopped{false} {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Constructing...", mClassNamePrefix);
    // Create pipe
    if (!mPipe->create()) {
        throw std::runtime_error(mClassNamePrefix + "Failed to create named pipe!");
    } else {
        logger.info("{} Successfully created named pipe!", mClassNamePrefix);
    }
    // Set main receiver thread
    std::promise<void> mainPromise;
    mBlockers.push_back(mainPromise.get_future());
    mThreads.push_back(std::thread(&TTTextBoxHandler::main, this, std::move(mainPromise)));
    mThreads.back().detach();
}

TTTextBoxHandler::~TTTextBoxHandler() {
    TTDiagnosticsLogger::getInstance().info("{} Destructing...", mClassNamePrefix);
    stop();
    for (auto &blocker : mBlockers) {
        blocker.wait();
    }
}

void TTTextBoxHandler::stop() {
    TTDiagnosticsLogger::getInstance().info("{} Forced stop...", mClassNamePrefix);
    mStopped.store(true);
}

bool TTTextBoxHandler::stopped() const {
    return mStopped.load();
}

void TTTextBoxHandler::main(std::promise<void> promise) {
    decltype(auto) logger = TTDiagnosticsLogger::getInstance();
    logger.info("{} Started textbox handler loop", mClassNamePrefix);
    if (mPipe->alive()) {
        try {
            for (auto i = TTTEXTBOX_RECEIVE_TRY_COUNT; i >= 0; --i) {
                if (stopped()) {
                    break;
                }
                TTTextBoxMessage message(TTTextBoxStatus::UNDEFINED, 0, nullptr);
                if (mPipe->receive(&message)) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(TTTEXTBOX_RECEIVE_TIMEOUT_MS));
                    continue;
                }

                switch (message.status) {
                    case TTTextBoxStatus::HEARTBEAT:
                        logger.info("{} Received heartbeat message", mClassNamePrefix);
                        break;
                    case TTTextBoxStatus::CONTACTS_SWITCH:
                    {
                        logger.info("{} Received contacts switch message", mClassNamePrefix);
                        size_t id = 0;
                        memcpy(&id, message.data, message.dataLength);
                        mCallbackContactsSwitch(id);
                        break;
                    }
                    case TTTextBoxStatus::MESSAGE:
                    {
                        logger.info("{} Received message", mClassNamePrefix);
                        std::string msg(message.data, message.dataLength);
                        mCallbackMessageSent(std::move(msg));
                        break;
                    }
                    case TTTextBoxStatus::UNDEFINED:
                    default:
                        logger.error("{} Received undefined message!", mClassNamePrefix);
                        throw std::runtime_error({});
                }
                i = TTTEXTBOX_RECEIVE_TRY_COUNT + 1;
            }
        } catch (...) {
            // ...
        }
    }
    stop();
    promise.set_value();
    logger.info("{} Completed textbox handler loop", mClassNamePrefix);
}