#pragma once
#include <iostream>
#include <string>

class TTContactsOutputStream {
public:
    TTContactsOutputStream() = default;
    virtual ~TTContactsOutputStream() {}
    TTContactsOutputStream(const TTContactsOutputStream&) = default;
    TTContactsOutputStream(TTContactsOutputStream&&) = default;
    constexpr TTContactsOutputStream& operator=(const TTContactsOutputStream&) = default;
    constexpr TTContactsOutputStream& operator=(TTContactsOutputStream&&) = default;

    virtual const TTContactsOutputStream& print(const char* cmessage) const {
        std::cout << cmessage;
        return *this;
    }

    virtual const TTContactsOutputStream& print(std::string message) const {
        std::cout << message;
        return *this;
    }

    virtual const TTContactsOutputStream& endl() const {
        std::cout << std::endl;
        return *this;
    }

    virtual const TTContactsOutputStream& flush() const {
        std::cout << std::flush;
        return *this;
    }
};