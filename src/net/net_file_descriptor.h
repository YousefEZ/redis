#ifndef INCLUDED_NET_FILE_DESCRIPTOR_H
#define INCLUDED_NET_FILE_DESCRIPTOR_H

#include <fcntl.h>
#include <unistd.h>

namespace net {
struct FileDescriptor {
    int m_fd{-1};

    FileDescriptor(int fd);

    FileDescriptor(const FileDescriptor&)            = delete;
    FileDescriptor& operator=(const FileDescriptor&) = delete;

    FileDescriptor(FileDescriptor&& other);
    FileDescriptor& operator=(FileDescriptor&& other);

    ~FileDescriptor();

    void as_non_blocking() const;
         operator int() const;
};
}  // namespace net
#endif
