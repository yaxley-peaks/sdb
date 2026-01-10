//
// Created by yax on 11/01/26.
//

#include <libsdb/process.hpp>
#include <libsdb/error.hpp>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

auto sdb::process::launch(std::filesystem::path path) -> std::unique_ptr<process> {
    pid_t pid;
    if ((pid = fork()) < 0) {
        error::send_errno("fork failed");
    }
    if (pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
            error::send_errno("tracing failed");
        }
        if (execlp(path.c_str(), path.c_str(), nullptr) < 0) {
            error::send_errno("exec failed");
        }
    }

    std::unique_ptr<process> proc(new process(pid, true));
    proc->wait_on_signal();
    return proc;
}

auto sdb::process::attach(pid_t pid) -> std::unique_ptr<process> {
    if (pid == 0) {
        error::send("Invalid PID");
    }

    if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
        error::send_errno("Could not attach");
    }

    std::unique_ptr<process> proc(new process(pid, false));
    proc->wait_on_signal();
    return proc;
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
    return reason;
}

sdb::process::~process() {
    if (this->pid() != 0) {
        int status;
        if (this->state() == process_state::running) {
            kill(this->pid(), SIGSTOP);
            waitpid(this->pid(), &status, 0);
        }
        ptrace(PTRACE_DETACH, this->pid(), nullptr, nullptr);
        kill(this->pid(), SIGCONT);

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
