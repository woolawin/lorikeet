#ifndef LK_STATE_MACHINE
#define LK_STATE_MACHINE

#include <vector>
#include <string>
#include <optional>

#include "ports.h"
#include "core_types.h"

class StateMachine {
    public:
    virtual TaxStrat tax_strat(const std::string& instr_name) = 0;
};

struct CommandInstr {
    std::string name;
    std::string path;

    bool operator==(const CommandInstr& other) const;
};

class RootStateMachine: public StateMachine {
    private:
    Env& env;
    Disk& disk;
    std::vector<CommandInstr> command_instrs;

    void load_cmd_instrs(const std::string& path);

    public:
    RootStateMachine(Env& env, Disk& disk) : env(env), disk(disk), command_instrs({}) {}

    std::optional<CommandInstr> get_cmd_instr(const std::string& name);

    void init();
    TaxStrat tax_strat(const std::string& instr_name);
};


#endif