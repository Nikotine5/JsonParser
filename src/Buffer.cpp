#include "Buffer.hpp"
#include <cerrno>
#include <cstring>
#include <system_error>
#include <unistd.h>

Buffer::Buffer(int fd) : m_buf_(s_capacity), m_fd_(fd) {}

//takes ownership of the string's bytes; nothing left to read from an fd, so
//eof is true from the start and ensure() never enters its read loop
Buffer::Buffer(std::string data)
    : m_buf_(data.begin(), data.end()),
      m_end_(m_buf_.size()),
      m_fd_(-1),
      m_eof_(true)
{}

Buffer::~Buffer() {if (m_fd_ >= 0) { ::close(m_fd_); }}

bool Buffer::ensure(std::size_t n)
{
    if (m_end_ - m_pos_ >= n) { return true; }
    if (m_eof_) { return false; }

    //slide whatever is left over to the front so we can read into the tail
    if (m_pos_ > 0) {
        std::size_t left = m_end_ - m_pos_;
        std::memmove(m_buf_.data(), m_buf_.data() + m_pos_, left);
        m_pos_ = 0;
        m_end_ = left;
    }

    while (m_end_ - m_pos_ < n && !m_eof_) {
        ssize_t got = ::read(m_fd_, m_buf_.data() + m_end_, m_buf_.size() - m_end_);

        if (got < 0) {
            if (errno == EINTR) continue;
            throw std::system_error(errno, std::generic_category(), "Buffer::ensure");
        }

        if (got == 0) { m_eof_ = true; break; }

        m_end_ += static_cast<std::size_t>(got);
    }
    
    return m_end_ - m_pos_ >= n;
}

//cast through unsigned char so a UTF-8 byte >0x7F doesn't come back negative
//and collide with the -1 end-of-input sentinel
int Buffer::peek()
{
    return ensure(1) ? static_cast<unsigned char>(m_buf_[m_pos_]) : -1;
}

int Buffer::peekNext()
{
    return ensure(2) ? static_cast<unsigned char>(m_buf_[m_pos_ + 1]) : -1;
}

int Buffer::advance()
{
    if (!ensure(1)) { return -1; }
    return static_cast<unsigned char>(m_buf_[m_pos_++]);
}

bool Buffer::eof() { return !ensure(1); }
