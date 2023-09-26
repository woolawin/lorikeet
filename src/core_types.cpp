#include "core_types.h"

TaxStrat value_strat() {
    return { .parse_strat = ParseStrat::Value, .block_function = BlockFunction::NA };
}

TaxStrat command_strat() {
    return { .parse_strat = ParseStrat::Command, .block_function = BlockFunction::Append };
}

TaxStrat branch_strat(std::vector<std::string> branch_instr) {
    return { .parse_strat = ParseStrat::Branch, .block_function = BlockFunction::Routine, .branch_instr = branch_instr };
}

TaxStrat custom_strat(BlockFunction block_func) {
    return { .parse_strat = ParseStrat::Custom, .block_function = block_func };
}

InstructionID RandomIDGenerator::new_instr_id() {
    return this->distribution(this->rng);
}