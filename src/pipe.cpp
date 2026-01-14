//
// Created by yax on 14/01/26.
//

#include <libsdb/pipe.hpp>
#include <libsdb/error.hpp>


#include <unistd.h>
#include <fcntl.h>

#include <utility>


sdb::pipe::pipe(bool close_on_exec) {
    if (pipe2(this->fds_, close_on_exec ? O_CLOEXEC : 0) < 0) {
        error::send_errno("Pipe creation failed");
    }
}

sdb::pipe::~pipe() {
    this->close_read();
    this->close_write();
}

auto sdb::pipe::release_read() -> int {
    return std::exchange(this->fds_[read_fd], -1);
}

auto sdb::pipe::release_write() -> int {
    return std::exchange(this->fds_[write_fd], -1);
}

auto sdb::pipe::close_read() -> void {
    if (this->fds_[read_fd] != -1) {
        close(this->fds_[read_fd]);
        this->fds_[read_fd] = -1;
    }
}

auto sdb::pipe::close_write() -> void {
    if (this->fds_[write_fd] != -1) {
        close(this->fds_[write_fd]);
        this->fds_[write_fd] = -1;
    }
}

auto sdb::pipe::read() const -> std::vector<std::byte> {
    char buf[1024];
    int chars_read;
    if ((chars_read = ::read(this->fds_[read_fd], buf, sizeof(buf))) < 0) {
        error::send_errno("Could not read from pipe");
    }
    const auto bytes = reinterpret_cast<std::byte *>(buf);
    return std::vector(bytes, bytes + chars_read);
}

auto sdb::pipe::write(const std::byte *from, const std::size_t bytes) const -> void {
    if (::write(this->fds_[write_fd], from, bytes) < 0) {
        error::send_errno("Could not write to pipe");
    }
}
