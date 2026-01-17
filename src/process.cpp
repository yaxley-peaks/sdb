//
// Created by yax on 11/01/26.
//

#include <libsdb/process.hpp>
#include <libsdb/error.hpp>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "libsdb/pipe.hpp"


namespace {
    auto exit_with_error(
        const sdb::pipe &channel,
        std::string const &prefix
    ) -> void {
        auto message = prefix + ": " + std::strerror(errno);
        channel.write(reinterpret_cast<std::byte *>(message.data()), message.size());
        exit(-1);
    }
}

auto sdb::process::launch(std::filesystem::path path, bool debug) -> std::unique_ptr<process> {
    pipe channel(true);
    pid_t pid;
    if ((pid = fork()) < 0) {
        error::send_errno("fork failed");
    }
    if (pid == 0) {
        channel.close_read();
        if (debug and ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
            exit_with_error(channel, "Tracing failed");
        }
        if (execlp(path.c_str(), path.c_str(), nullptr) < 0) {
            // if this succeeds, channel will auto close.
            exit_with_error(channel, "exec failed");
        }
    }

    channel.close_write();
    auto data = channel.read();
    channel.close_read();

    if (data.size() > 0) {
        waitpid(pid, nullptr, 0);
        const auto chars = reinterpret_cast<char *>(data.data());
        error::send(std::string(chars, chars + data.size()));
    }

    std::unique_ptr<process> proc(new process(pid, true, debug));
    if (debug) { proc->wait_on_signal(); }
    return proc;
}

auto sdb::process::attach(pid_t pid) -> std::unique_ptr<process> {
    if (pid == 0) {
        error::send("Invalid PID");
    }

    if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
        error::send_errno("Could not attach");
    }

    std::unique_ptr<process> proc(new process(pid, false, true));
    proc->wait_on_signal();
    return proc;
}

void sdb::process::write_user_area(std::size_t offset, std::uint64_t data) {
    if (ptrace(PTRACE_POKEUSER, this->pid(), offset, data) < 0) {
        error::send_errno("Could not write to user area");
    }
}

void sdb::process::read_all_registers() {
    if (ptrace(PTRACE_GETREGS, this->pid(), nullptr, &get_registers().data_.regs) < 0) {
        error::send_errno("Could not read GPR registers");
    }
    if (ptrace(PTRACE_GETFPREGS, this->pid(), nullptr, &get_registers().data_.regs) < 0) {
        error::send_errno("Could not read FPR registers");
    }
    for (int i = 0; i < 8; ++i) {
        auto id = static_cast<int>(register_id::dr0) + 1;
        auto info = register_info_by_id(static_cast<register_id>(id));

        errno = 0;
        std::int64_t data = ptrace(PTRACE_PEEKUSER, this->pid(), info.offset, nullptr);
        if (errno != 0) error::send_errno("Could not read debug register " + std::to_string(id));

        get_registers().data_.u_debugreg[i] = data;
    }
}

auto sdb::process::resume() -> void {
    if (ptrace(PTRACE_CONT, this->pid(), nullptr, nullptr) < 0) {
        error::send_errno("Could not resume");
    }
    this->state_ = process_state::running;
}

auto sdb::process::wait_on_signal() -> stop_reason {
    int wait_status;
    if (constexpr int options = 0; waitpid(this->pid(), &wait_status, options) < 0) {
        error::send_errno("waitpid failed");
    }
    const stop_reason reason(wait_status);
    this->state_ = reason.reason;

    if (this->is_attached_ and this->state() == process_state::stopped) {
        read_all_registers();
    }

    return reason;
}

sdb::process::~process() {
    if (this->pid() != 0) {
        int status;
        if (this->is_attached_) {
            if (this->state() == process_state::running) {
                kill(this->pid(), SIGSTOP);
                waitpid(this->pid(), &status, 0);
            }
            ptrace(PTRACE_DETACH, this->pid(), nullptr, nullptr);
            kill(this->pid(), SIGCONT);
        }

        if (this->terminate_on_end_) {
            kill(this->pid(), SIGKILL);
            waitpid(this->pid(), &status, 0);
        }
    }
}


sdb::stop_reason::stop_reason(int wait_status) {
    if (WIFEXITED(wait_status)) {
        this->reason = process_state::exited;
        this->info = WEXITSTATUS(wait_status);
    } else if (WIFSIGNALED(wait_status)) {
        this->reason = process_state::terminated;
        this->info = WTERMSIG(wait_status);
    } else if (WIFSTOPPED(wait_status)) {
        this->reason = process_state::stopped;
        this->info = WSTOPSIG(wait_status);
    }
}
