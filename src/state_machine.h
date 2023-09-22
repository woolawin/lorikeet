#ifndef LK_STATE_MACHINE
#define LK_STATE_MACHINE

#include <vector>
#include <string>

#include "ports.h"
#include "core_types.h"

class StateMachine {
    public:
    virtual TaxStrat tax_strat(const std::string& instr_name) = 0;
};

struct CommandInstr {
    std::string name;
    std::string path;
};

class DefaultStateMachine: public StateMachine {
    private:
    Env& env;
    Disk& disk;
    std::vector<CommandInstr> command_instrs;

    public:
    DefaultStateMachine(Env& env, Disk& disk) : env(env), disk(disk), command_instrs({}) {
    }

    void init(const std::string& cwd);
    TaxStrat tax_strat(const std::string& instr_name);
};


#endif