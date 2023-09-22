#ifndef LK_CORE_TYPES
#define LK_CORE_TYPES

#include <string>
#include <vector>

enum class ParseStrat {
    Value,
    Command,
    Branch,
    Custom
};

enum class BlockFunction {
    NA,
    Append,
    Routine
};

struct TaxStrat {
    ParseStrat parse_strat;
    BlockFunction block_function;
    std::vector<std::string> branch_instr;
};

TaxStrat value_strat();
TaxStrat command_strat();
TaxStrat branch_strat(std::vector<std::string> branch_instr);
TaxStrat custom_strat(BlockFunction block_func);


#endif