#include "taxscan.h"

void DefaultStateMachine::init(const std::string& cwd) {
    const std::string path = this->env.var("PATH");
    for (File file : this->disk.ls(cwd)) {
        if (file.can_execute) {
            this->command_instrs.push_back({ .name = file.name, .path = file.path });
        }
    }
}

TaxStrat DefaultStateMachine::tax_strat(const std::string& instr_name) {
    return value_strat();
}

