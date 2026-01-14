//
// Created by yax on 10/01/26.
//

#include <csignal>
#include <sys/types.h>
#include <fstream>

#include <catch2/catch_test_macros.hpp>
#include <libsdb/process.hpp>

#include "libsdb/error.hpp"

using namespace sdb;


namespace {
    auto process_exists(const pid_t pid) -> bool {
        const auto ret = kill(pid, 0);
        return  ret != -1 && errno != ESRCH;
    }
    auto get_process_status(const pid_t pid) -> char {
        std::ifstream stat("/proc/" + std::to_string(pid) + "/stat");
        std::string data;
        std::getline(stat, data);
        const auto index_of_last_parenthesis = data.rfind(')');
        const auto index_of_status_indicator = index_of_last_parenthesis + 2;
        return data[index_of_status_indicator];
    }
}

TEST_CASE("validate environment") {
    REQUIRE(true);
}

TEST_CASE("process::launch success", "[process]") {
    const auto proc = process::launch("yes");
    REQUIRE(process_exists(proc->pid()));
}

TEST_CASE("process::launch no such process", "[process]") {
    REQUIRE_THROWS_AS(process::launch("no_such_program"), error);
}


TEST_CASE("process::attach success", "[process]") {
    const auto pid = process::launch("targets/run_endlessly", false);
    auto proc = process::attach(pid->pid());
    REQUIRE(get_process_status(pid->pid()) == 't');
}