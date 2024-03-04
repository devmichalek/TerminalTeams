#pragma once
#include <string>

class TTEmulator {
public:
    explicit TTEmulator(size_t width, size_t height);
    void print(std::string message) const;
    void println(std::string message) const;
    void flush() const;
    void clear() const;
    size_t getWidth() const;
    size_t getHeight() const;
private:
    size_t mWidth;
    size_t mHeight;
};