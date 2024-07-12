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

    virtual TTUtilsOutputStream& print(const char* cmessage) {
        std::cout << cmessage;
        return *this;
    }

    virtual TTUtilsOutputStream& print(std::string message) {
        std::cout << message;
        return *this;
    }

    virtual TTUtilsOutputStream& endl() {
        std::cout << std::endl;
        return *this;
    }

    virtual TTUtilsOutputStream& flush() {
        std::cout << std::flush;
        return *this;
    }

    virtual TTUtilsOutputStream& clear() {
        print("\033[2J\033[1;1H").flush();
        return *this;
    }
};