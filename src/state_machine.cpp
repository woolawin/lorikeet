#include <iostream>

#include "state_machine.h"


bool CommandInstr::operator==(const CommandInstr& other) const {
    return this->name == other.name
        && this->path == other.path
        && this->id == other.id;
}

std::ostream& operator<<(std::ostream& os, const CommandInstr& instr) {
    os << "{ id=" << instr.id << ", name=\"" << instr.name << "\", path=\"" << instr.path << "\" }" << std::endl;
    return os;
}

std::vector<std::string> split_paths(std::string path);


void RootStateMachine::init() {
    std::vector<std::string> paths = split_paths(this->env.var("PATH"));
    for (const std::string& path : paths) {
        this->load_cmd_instrs(path);
    }
}

std::vector<std::string> split_paths(std::string path) {
    std::vector<std::string> paths;
    size_t pos = 0;
    while ((pos = path.find(":")) != std::string::npos) {
        paths.push_back(path.substr(0, pos));
        path.erase(0, pos + 1);
    }
    return paths;
}

void RootStateMachine::load_cmd_instrs(const std::string& path) {
    for (File file : this->disk.ls(path)) {
        if (file.can_execute) {
            this->command_instrs.push_back({ .id = this->id_gen.new_instr_id(), .name = file.name, .path = file.path });
        }
    }
}

TaxStrat RootStateMachine::tax_strat(const std::string& instr_name) {
    for (const CommandInstr& instr : this->command_instrs) {
        if (instr.name == instr_name) {
            return command_strat();
        }
    }
    return value_strat();
}

std::optional<InstructionID> RootStateMachine::find_instr(const std::string& name) {
    const std::optional<CommandInstr> cmd_opt = this->get_cmd_instr(name);
    if (cmd_opt.has_value()) {
        return cmd_opt.value().id;
    }
    return std::nullopt;
}

std::optional<CommandInstr> RootStateMachine::get_cmd_instr(const std::string& instr_name) {
    for (const CommandInstr& instr : this->command_instrs) {
        if (instr.name == instr_name) {
            return instr;
        }
    }
    return std::nullopt;
}