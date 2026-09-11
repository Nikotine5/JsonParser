#ifndef BUFFER_HPP
#define BUFFER_HPP
#include <cstddef>
#include <vector>

//Chunked reader over a file descriptor. Hands out one byte at a time and
//refills from the fd behind the scenes, so the caller never sees chunk edges.
//Does not own the fd.
class Buffer{
private:
    static constexpr std::size_t s_capacity = 1 << 16;

    std::vector<char> m_buf_;
    std::size_t m_pos_ = 0;   //next byte to hand out
    std::size_t m_end_ = 0;   //one past the last valid byte
    int m_fd_;
    bool m_eof_ = false;      //::read returned 0

    //guarantee n readable bytes are sitting at m_pos_, refilling if needed
    bool ensure(std::size_t n);
public:
    explicit Buffer(int fd);

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    //all three return -1 at true end of input, never '\0', because a JSON
    //string may legally contain a 0 byte via a \u0000 escape
    int peek();
    int peekNext();
    int advance();

    bool eof();
    std::vector<char> getBuf() const { return m_buf_; }
    std::size_t getPos() const { return m_pos_; }
};
#endif // BUFFER_HPP
