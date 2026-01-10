//
// Created by yax on 10/01/26.
//


#include <iostream>
#include <unistd.h>
#include <sys/ptrace.h>


namespace {
    auto attach(int argc, const char** argv) -> pid_t {
        pid_t pid = 0;
        if (argc == 3 && argv[1] == std::string_view("-p")) {
            pid = static_cast<pid_t>(std::strtol(argv[2], nullptr, 0));
            if (pid <= 0) {
                std::cerr << "Invalid PID\n";
                return -1;
            }
            if (ptrace(PTRACE_ATTACH, pid, /*ADDR=*/nullptr, /*DATA=*/nullptr) < 0) {
                std::perror("Could not attach");
                return -1;
            }
        } else {
        const char* program_path = argv[1];
            if ((pid = fork()) < 0) {
                std::perror("Fork failed");
                return -1;
            }
            if (pid == 0) { // In child
                if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
                    std::perror("Tracing failed");
                    return -1;
                }
                if (execlp(program_path, program_path, nullptr) <0) {
                    std::perror("Exec failed");
                    return -1;
                }
            }
        }
        return  pid;
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cerr << "No arguments given\n";
        return -1;
    }

    pid_t pid = attach(argc, argv);
}
