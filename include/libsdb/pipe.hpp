//
// Created by yax on 14/01/26.
//

#ifndef SDB_PIPE_HPP
#define SDB_PIPE_HPP

#include <vector>
#include <cstddef>

namespace sdb {
    class pipe {
    public:
        explicit pipe(bool close_on_exec);

        ~pipe();

        auto get_read() const -> int {
            return this->fds_[read_fd];
        }

        auto get_write() const -> int { return this->fds_[write_fd]; }

        auto release_read() -> int;

        auto release_write() -> int;

        auto close_read() -> void;

        auto close_write() -> void;


        auto read() const -> std::vector<std::byte>;
        auto write(const std::byte* from, std::size_t bytes) const -> void;

    private:
        static constexpr unsigned read_fd = 0;
        static constexpr unsigned write_fd = 1;
        int fds_[2];
    };
} // sdb

#endif //SDB_PIPE_HPP
