#include "TTEngine.hpp"
#include "TTUtilsSignals.hpp"

class TTUtilsSharedMemDebug : public TTUtilsSharedMem {
    bool create() override {

    }
    bool open(long attempts, long timeoutMs) override {

    }
    bool receive(void* message, long attempts, long timeoutMs) override {

    }
    bool send(const void* message, long attempts, long timeoutMs) override {

    }
    bool alive() const override {

    }
    bool destroy() override {

    }
};

class TTUtilsMessageQueueDebug : public TTUtilsMessageQueue {
public:
    bool create() override {

    }
    bool open(long attempts, long timeoutMs) override {

    }
    bool alive() const override {

    }
    bool receive(char* message, long attempts, long timeoutMs) override {

    }
    bool send(const char* message, long attempts, long timeoutMs) override {

    }
};

class TTUtilsNamedPipeDebug : public TTUtilsNamedPipe {
public:
    bool alive() const override {

    }
    bool create(long attempts, long timeoutMs) override {

    }
    bool open(long attempts, long timeoutMs) override {

    }
    bool receive(char* message) override {

    }
    bool send(const char* message) override {

    }
};

class TTContactsSettings {
public:
    std::shared_ptr<TTUtilsSharedMem> getSharedMemory() const override {

    }
};

class TTEngineSettingsDebug : public TTEngineSettings {
public:
    const TTAbstractFactory& getAbstractFactory() const override {
    
    }
};

// Application
std::unique_ptr<TTEngine> application;
LOG_DECLARE("tteams-engine");

void signalInterruptHandler(int) {
    if (application) {
        LOG_WARNING("Stopping due to caught signal");
        application->stop();
    }
}

int main(int argc, char** argv) {
    try {
        TTUtilsSignals signals(std::make_shared<TTUtilsSyscall>());
        signals.setup(signalInterruptHandler, { SIGINT, SIGTERM, SIGSTOP });
        // Run application
        TTEngineSettings settings(argc, argv);
        application = std::make_unique<TTEngine>(settings);
        LOG_INFO("Engine initialized");
        try {
            if (!application->isStopped()) {
                application->run();
            } else {
                LOG_WARNING("Application was shut down all of a sudden");
            }
        } catch (const std::exception& exp) {
            LOG_ERROR("Exception captured: {}", exp.what());
            throw;
        }
    } catch (const std::exception& exp) {
        LOG_ERROR("Exception captured: {}", exp.what());
    }
    application.reset();
    LOG_INFO("Successfully flushed all logs");
    return 0;
}