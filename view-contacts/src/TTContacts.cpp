#include "TTContacts.hpp"
#include <chrono>
#include <thread>
#include <cstring>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

TTContacts::TTContacts(TTContactsSettings settings,
    TTContactsCallbackQuit callbackQuit,
    TTContactsCallbackOutStream& callbackOutStream) :
        mCallbackQuit(callbackQuit),
        mCallbackOutStream(callbackOutStream),
        mSharedMessage(nullptr),
        mDataProducedSemaphore(nullptr),
        mDataConsumedSemaphore(nullptr),
        mTerminalWidth(settings.getTerminalWidth()),
        mTerminalHeight(settings.getTerminalHeight()) {
    const std::string classNamePrefix = "TTContacts: ";
    auto sharedMemoryName = settings.getSharedMemoryName();
    std::string dataProducedSemName = sharedMemoryName + std::string(TTCONTACTS_DATA_PRODUCED_POSTFIX);
    std::string dataConsumedSemName = sharedMemoryName + std::string(TTCONTACTS_DATA_CONSUMED_POSTFIX);

    errno = 0;
    for (auto attempt = TTCONTACTS_SEMAPHORES_READY_TRY_COUNT; attempt > 0; --attempt) {
        
        if ((mDataProducedSemaphore = sem_open(dataProducedSemName.c_str(), 0)) != SEM_FAILED) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(TTCONTACTS_SEMAPHORES_READY_TIMEOUT_MS));
    }

    if (mDataProducedSemaphore == SEM_FAILED) {
        throw std::runtime_error(classNamePrefix + "Failed to open data produced semaphore, errno=" + std::to_string(errno));
    }

    errno = 0;
    if ((mDataConsumedSemaphore = sem_open(dataConsumedSemName.c_str(), 0)) == SEM_FAILED) {
        throw std::runtime_error(classNamePrefix + "Failed to open data consumed semaphore, errno=" + std::to_string(errno));
    }

    int fd = shm_open(sharedMemoryName.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        throw std::runtime_error(classNamePrefix + "Failed to open shared object, errno=" + std::to_string(errno));
    }

    void* rawPointer = mmap(nullptr, sizeof(TTContactsMessage), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    mSharedMessage = new(rawPointer) TTContactsMessage;
}

void TTContacts::run() {
    if (!mSharedMessage) {
        return;
    }
    
    try {
        while (!mCallbackQuit()) {
            // Wait for the other process to produce the data
            {
                int result = 0;
                for (auto attempt = TTCONTACTS_DATA_PRODUCE_TRY_COUNT; attempt >= 0; --attempt) {
                    struct timespec ts;
                    if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
                        result = -1;
                        break; // Hard failure
                    }
                    ts.tv_sec += TTCONTACTS_DATA_PRODUCE_TIMEOUT_S;
                    result = sem_timedwait(mDataProducedSemaphore, &ts);
                    if (result != -1) {
                        break; // Success
                    } else if (errno == EINTR) {
                        continue; // Soft failure
                    }
                }
                if (result == -1) {
                    break;
                }
            }

            TTContactsMessage newMessage;
            memcpy(&newMessage, mSharedMessage, sizeof(newMessage));

            if (sem_post(mDataConsumedSemaphore) == -1) {
                break;
            }
            
            if (handle(newMessage)) {
                refresh();
            }
        }
    } catch (...) {
        // ...
    }
}

bool TTContacts::handle(const TTContactsMessage& message) {
    if (message.status == TTContactsStatus::HEARTBEAT) {
        // Nothing to be done
        return false;
    } else {
        if (message.status == TTContactsStatus::ACTIVE && message.id >= mContacts.size()) {
            std::string nickname(message.data, message.data + message.dataLength);
            auto newContact = std::make_tuple(message.id, nickname, message.status);
            mContacts.push_back(newContact);
        } else {
            auto& contact = mContacts[message.id];
            std::get<2>(contact) = message.status;
        }
    }
    return true;
}

void TTContacts::refresh() {
    mCallbackOutStream << "\033[2J\033[1;1H" << std::flush; // Clear window
    const std::array<std::string, 8> statuses = { "", "?", "<", "<?", "@", "@?", "!?", "<!?" };
    for (auto &contact : mContacts) {
        mCallbackOutStream << "#" << std::get<0>(contact);
        mCallbackOutStream << " " << std::get<1>(contact);
        mCallbackOutStream << " " << statuses[std::get<2>(contact)];
        mCallbackOutStream << std::endl;
    }
}
