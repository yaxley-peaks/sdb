//
// Created by yax on 10/01/26.
//


#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <sstream>
#include <cstring>

#include <editline/readline.h>

#include <libsdb/process.hpp>

#include "libsdb/error.hpp"


namespace {
    auto split(const std::string_view str, char delimiter) -> std::vector<std::string> {
        std::vector<std::string> out{};
        std::stringstream ss{std::string{str}};
        std::string item;

        while (std::getline(ss, item, delimiter)) {
            out.push_back(item);
        }
        return out;
    }

    auto is_prefix(const std::string_view str, std::string_view of) -> bool {
        if (str.size() > of.size()) return false;
        return std::equal(str.begin(), str.end(), of.begin());
    }

    auto print_stop_reason(
        const sdb::process &process, const sdb::stop_reason reason) -> void {
        std::cout << "Process " << process.pid() << ' ';
        switch (reason.reason) {
            case sdb::process_state::exited:
                std::cout << "exited with status "
                        << static_cast<int>(reason.info);
                break;
            case sdb::process_state::terminated:
                std::cout << "terminated with signal "
                        << sigabbrev_np(reason.info);
                break;
            case sdb::process_state::stopped:
                std::cout << "stopped with signal " << sigabbrev_np(reason.info);
                break;
            case sdb::process_state::running:
                break;
        }
        std::cout << std::endl;
    }

    auto handle_command(const std::unique_ptr<sdb::process> &process, const std::string_view line) -> void {
        const auto args = split(line, ' ');
        const auto &command = args[0];
        if (is_prefix(command, "continue")) {
            process->resume();
            const auto reason = process->wait_on_signal();
            print_stop_reason(*process, reason);
        } else {
            std::cerr << "Unknown command\n";
        }
    }

    auto attach(const int argc, const char **argv) -> std::unique_ptr<sdb::process> {
        if (argc == 3 && argv[1] == std::string_view("-p")) {
            const auto pid = static_cast<pid_t>(std::strtol(argv[2], nullptr, 0));
            return sdb::process::attach(pid);
        } else {
            const char *program_path = argv[1];
            return sdb::process::launch(program_path);
        }
    }


    void main_loop(const std::unique_ptr<sdb::process> &process) {
        char *line = nullptr;
        while ((line = readline("sdb> ")) != nullptr) {
            std::string line_str;
            if (line == std::string_view("")) {
                free(line);
                if (history_length > 0) {
                    line_str = history_list()[history_length - 1]->line;
                }
            } else {
                line_str = line;
                add_history(line);
                free(line);
            }

            if (!line_str.empty()) {
                try {
                    handle_command(process, line_str);
                } catch (const sdb::error &err) {
                    std::cout << err.what() << '\n';
                }
            }
        }
    }
}

int main(const int argc, const char **argv) {
    if (argc == 1) {
        std::cerr << "No arguments given\n";
        return -1;
    }

    try {
        const auto process = attach(argc, argv);
        main_loop(process);
    } catch (const sdb::error &err) {
        std::cout << err.what() << '\n';
    }
}
