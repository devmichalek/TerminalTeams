#pragma once
#include <iostream>
#include <string>

class TTUtilsOutputStream {
public:
    TTUtilsOutputStream() = default;
    virtual ~TTUtilsOutputStream() {}
    TTUtilsOutputStream(const TTUtilsOutputStream&) = default;
    TTUtilsOutputStream(TTUtilsOutputStream&&) = default;
    TTUtilsOutputStream& operator=(const TTUtilsOutputStream&) = default;
    TTUtilsOutputStream& operator=(TTUtilsOutputStream&&) = default;

    virtual const TTUtilsOutputStream& print(const char* cmessage) const {
        std::cout << cmessage;
        return *this;
    }

    virtual const TTUtilsOutputStream& print(std::string message) const {
        std::cout << message;
        return *this;
    }

    virtual const TTUtilsOutputStream& endl() const {
        std::cout << std::endl;
        return *this;
    }

    virtual const TTUtilsOutputStream& flush() const {
        std::cout << std::flush;
        return *this;
    }
};