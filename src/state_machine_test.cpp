#include <gtest/gtest.h>

#include "state_machine.h"

class TestDisk: public Disk {
    public:
    std::vector<File> ls(const std::string& path) {
        if (path == "/usr/bin") {
            return {
                {
                    .path = "/usr/bin/echo",
                    .name = "echo",
                    .can_execute = true
                },
                {
                    .path = "/usr/bin/cat",
                    .name = "cat",
                    .can_execute = true
                }
            };
        }
        if (path == "/usr/local/bin") {
            return {
                {
                    .path = "/usr/local/bin/make",
                    .name = "make",
                    .can_execute = true
                },
                {
                    .path = "/usr/local/bin/textdata",
                    .name = "make",
                    .can_execute = false
                }
            };
        }
        return {};
    }
};

class TestEnv: public Env {
    public:
     std::string var(const std::string& name) {
        if (name == "PATH") {
            return "/usr/bin:/usr/local/bin:/home";
        }
        return "";
     }
};

TestDisk disk = TestDisk();
TestEnv env = TestEnv();
SequentialIDGenerator id_gen = SequentialIDGenerator();

TEST(Line, LoadCommandsInPath) {

    RootStateMachine machine = RootStateMachine(env, disk, id_gen);

    machine.init();

    CommandInstr instr = { .id = 1, .name = "echo", .path = "/usr/bin/echo" };
    EXPECT_EQ(machine.get_cmd_instr("echo").value(), instr);

    instr = { .id = 3, .name = "make", .path = "/usr/local/bin/make" };
    EXPECT_EQ(machine.get_cmd_instr("make").value(), instr);

    EXPECT_EQ(machine.get_cmd_instr("non_existant"), std::nullopt);
    EXPECT_EQ(machine.get_cmd_instr("textdata"), std::nullopt);
}

