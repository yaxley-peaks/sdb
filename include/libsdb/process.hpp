//
// Created by yax on 11/01/26.
//

#ifndef SDB_PROCESS_HPP
#define SDB_PROCESS_HPP

#include <filesystem>
#include <memory>
#include <sys/types.h>

namespace sdb {
    enum class process_state {
        stopped,
        running,
        exited,
        terminated,
    };

    class process {
    public:
        static auto launch(std::filesystem::path path) -> std::unique_ptr<process>;

        static auto attach(pid_t pid) -> std::unique_ptr<process>;

        auto resume() -> void;

        auto wait_on_signal() -> void;

        [[nodiscard]] auto pid() const -> pid_t { return pid_; }

        [[nodiscard]] auto state() const -> process_state { return state_; }

        process() = delete;

        process(const process &) = delete;

        process &operator=(const process &) = delete;

        ~process();

    private:
        pid_t pid_ = 0;
        bool terminate_on_end_ = true;
        process_state state_ = process_state::stopped;

        process(pid_t pid, bool terminate_on_end)
            : pid_(pid), terminate_on_end_(terminate_on_end) {
        }
    };
}

#endif //SDB_PROCESS_HPP
