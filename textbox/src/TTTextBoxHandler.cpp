#include "TTTextBoxHandler.hpp"
#include "TTTextBoxSettings.hpp"
#include "TTTextBoxMessage.hpp"
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

TTTextBoxHandler::TTTextBoxHandler(std::string uniqueName,
    TTTextBoxCallbackMessageSent callbackMessageSent,
    TTTextBoxCallbackContactSwitch callbackContactsSwitch) :
        mCallbackMessageSent(callbackMessageSent),
        mCallbackContactsSwitch(callbackContactsSwitch),
        mNamedPipeDescriptor(-1),
        mNamedPipePath(TTTextBoxSettings::getPipePath(uniqueName)),
        mStopped{false} {
    const std::string classNamePrefix = "TTTextBoxHandler: ";
    errno = 0;
    if (mkfifo(mNamedPipePath.c_str(), 0666) < 0) {
        throw std::runtime_error(classNamePrefix + "Failed to create named pipe, errno=" + std::to_string(errno));
    }

    mNamedPipeDescriptor = open(mNamedPipePath.c_str(), O_RDONLY);
    if (mNamedPipeDescriptor == -1) {
        throw std::runtime_error(classNamePrefix + "Failed to open named pipe, errno=" + std::to_string(errno));
    }

    // Set main receiver thread
    std::promise<void> mainPromise;
    mBlockers.push_back(mainPromise.get_future());
    mThreads.push_back(std::thread(&TTTextBoxHandler::main, this, std::move(mainPromise)));
    mThreads.back().detach();
}

TTTextBoxHandler::~TTTextBoxHandler() {
    stop();
    for (auto &blocker : mBlockers) {
        blocker.wait();
    }
    close(mNamedPipeDescriptor);
    unlink(mNamedPipePath.c_str());
}

void TTTextBoxHandler::stop() {
    mStopped.store(true);
}

bool TTTextBoxHandler::stopped() const {
    return mStopped.load();
}

void TTTextBoxHandler::main(std::promise<void> promise) {
    try {
        for (auto i = TTTEXTBOX_RECEIVE_TRY_COUNT; i >= 0; --i) {
            if (stopped()) {
                break;
            }
            TTTextBoxMessage message(TTTextBoxStatus::UNDEFINED, 0, nullptr);
            if (read(mNamedPipeDescriptor, &message, sizeof(TTTextBoxMessage)) < 0) {
                std::this_thread::sleep_for(std::chrono::milliseconds(TTTEXTBOX_RECEIVE_TIMEOUT_MS));
                continue;
            }

            switch (message.status) {
                case TTTextBoxStatus::HEARTBEAT:
                    break;
                case TTTextBoxStatus::CONTACTS_SWITCH:
                {
                    size_t id = 0;
                    memcpy(&id, message.data, message.dataLength);
                    mCallbackContactsSwitch(id);
                    break;
                }
                case TTTextBoxStatus::MESSAGE:
                {
                    std::string msg(message.data, message.dataLength);
                    mCallbackMessageSent(std::move(msg));
                    break;
                }
                case TTTextBoxStatus::UNDEFINED:
                default:
                    throw std::runtime_error({});
            }
            i = TTTEXTBOX_RECEIVE_TRY_COUNT + 1;
        }
    } catch (...) {
        // ...
    }
    stop();
    promise.set_value();
}