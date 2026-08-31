#include "Buffer.hpp"

Buffer::Buffer(int fd) : m_fd_(fd) {}

template <class... Args>
void Buffer::append(std::format_string<Args...> fmt, Args&&... args)
{
    std::format_to(std::back_inserter(m_buf_), fmt, std::forward<Args>(args)...);
    if (m_buf_.size() >= g_flushAt) { flush(); }
}

void Buffer::flush() {
    const char* ptr = m_buf_.data();
    size_t left = m_buf_.size();

    while (left > 0) {
        ssize_t written = ::write(m_fd_, ptr, left);

        if (written < 0) {
            if (errno == EINTR) continue;
            throw std::system_error(errno, std::generic_category());
        }

        ptr += written;
        left -= static_cast<size_t>(written);
        m_buf_.clear();
    }
}

Buffer::~Buffer() { try { flush(); } catch(...){} }