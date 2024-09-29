#pragma once
#include "TTUtilsSyscall.hpp"
#include "TTDiagnosticsLogger.hpp"
#include <memory>
#include <functional>
#include <vector>

class TTUtilsSignals {
public:
    TTUtilsSignals(std::shared_ptr<TTUtilsSyscall> syscall) : mSyscall(std::move(syscall)) {}
    virtual ~TTUtilsSignals() = default;
    TTUtilsSignals(const TTUtilsSignals&) = delete;
    TTUtilsSignals(TTUtilsSignals&&) = delete;
    TTUtilsSignals& operator=(const TTUtilsSignals&) = delete;
    TTUtilsSignals& operator=(TTUtilsSignals&&) = delete;

    virtual void setup(std::function<void(int)> handler, std::vector<int> signums) {
        LOG_INFO("Signal handling setup");
        struct sigaction signalAction;
        memset(&signalAction, 0, sizeof(signalAction));
        typedef void function_t(int) ;
        signalAction.sa_handler = handler.target<function_t>();
        mSyscall->sigfillset(&signalAction.sa_mask);
        for (const auto &signum : signums) {
            LOG_INFO("Setting action for the signal number={}", signum);
            mSyscall->sigaction(signum, &signalAction, nullptr);
        }
    }

    virtual void block(std::vector<int> signums) {
        sigset_t mask;
        mSyscall->sigemptyset(&mask);
        for (const auto &signum : signums) {
            LOG_INFO("Blocking signal number={} for the current thread", signum);
            mSyscall->sigaddset(&mask, signum);
        }
        mSyscall->pthread_sigmask(SIG_BLOCK, &mask, nullptr);
    }

    virtual void unblock(std::vector<int> signums) {
        sigset_t mask;
        mSyscall->sigemptyset(&mask);
        for (const auto &signum : signums) {
            LOG_INFO("Unblocking signal number={} for the current thread", signum);
            mSyscall->sigaddset(&mask, signum);
        }
        mSyscall->pthread_sigmask(SIG_UNBLOCK, &mask, nullptr);
    }

private:
    std::shared_ptr<TTUtilsSyscall> mSyscall;
};
