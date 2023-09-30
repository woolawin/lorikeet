#ifndef LK_STATE_MACHINE
#define LK_STATE_MACHINE

#include <iostream>
#include <vector>
#include <string>
#include <optional>

#include "ports.h"
#include "core_types.h"
#include "cmd_instr.h"

class StateMachine {
    public:
    virtual std::optional<InstructionID> find_instr(const std::string& name) = 0;
    virtual TaxStrat tax_strat(InstructionID instr) = 0;
};

class RootStateMachine: public StateMachine {
    private:
    Env& env;
    Disk& disk;
    IDGenerator& id_gen;
    std::vector<CommandInstr> command_instrs;

    void load_cmd_instrs(const std::string& path);

    public:
    RootStateMachine(Env& env, Disk& disk, IDGenerator& id_gen) :
        env(env),
        disk(disk),
        id_gen(id_gen),
        command_instrs({}) {}

    std::optional<CommandInstr> get_cmd_instr(const std::string& name);
    InstructionID new_instr_id();

    void init();
    std::optional<InstructionID> find_instr(const std::string& name);
    TaxStrat tax_strat(InstructionID instr);
};

class SequentialIDGenerator: public IDGenerator {
    private:
    size_t index = 0;

    public:
    InstructionID new_instr_id() {
        this->index++;
        return this->index;
    }
};

#endif