#pragma once
#include <iostream>
#include <string>

class TTUtilsInputStream {
public:
    TTUtilsInputStream() = default;
    virtual ~TTUtilsInputStream() {}
    TTUtilsInputStream(const TTUtilsInputStream&) = default;
    TTUtilsInputStream(TTUtilsInputStream&&) = default;
    TTUtilsInputStream& operator=(const TTUtilsInputStream&) = default;
    TTUtilsInputStream& operator=(TTUtilsInputStream&&) = default;

    virtual TTUtilsInputStream& readline(std::string& output) {
        std::getline(std::cin, output);
        return *this;
    }
};
