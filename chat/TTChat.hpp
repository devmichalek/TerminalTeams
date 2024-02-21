#pragma once
#include <iostream>

class TTChat {
public:
    explicit TTChat(size_t width, size_t height);
    void run();
private:
    void clear();
    void clearWindow() const;
};

