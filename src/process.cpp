//
// Created by yax on 11/01/26.
//

#include <libsdb/process.hpp>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

auto sdb::process::launch(std::filesystem::path path) -> std::unique_ptr<process> {

    pid_t pid;
    if ((pid = fork()) < 0) {
        // ERROR: Fork failed.
    }
    if (pid == 0) {
        if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
            // ERROR: Tracing failed.
        }
        if (execlp(path.c_str(), path.c_str(), nullptr) < 0) {
            // ERROR: exec failed.
        }
    }

    std::unique_ptr<process> proc (new process(pid, true));
    proc->wait_on_signal();
    return proc;
}

auto sdb::process::attach(pid_t pid) -> std::unique_ptr<process> {

    if (pid == 0) {
        // ERROR: Invalid PID.
    }

    if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr)< 0) {
        // ERROR: Could not attach.
    }

    std::unique_ptr<process> proc(new process(pid, false));
    proc->wait_on_signal();
    return proc;
}

auto sdb::process::resume() -> void {
}

auto sdb::process::wait_on_signal() -> void {
}

sdb::process::~process() = default;
