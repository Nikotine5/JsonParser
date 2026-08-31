#ifndef BUFFER_HPP
#define BUFFER_HPP
#include <vector>
#include <fstream>
#include <iostream>
#include <cstddef>
#include <format>

class Buffer{
private:
    static constexpr std::size_t g_flushAt = 1 << 16;
    std::string m_buf_;
    int m_fd_;
public:
    explicit Buffer(int fd);
    template <class... Args>
    void append(std::format_string<Args...> fmt, Args&&... args);
    void flush();
    ~Buffer();
};
#endif // BUFFER_HPP