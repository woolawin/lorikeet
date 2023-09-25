#ifndef LK_CORE_TYPES
#define LK_CORE_TYPES

#include <string>
#include <vector>
#include <random>

typedef uint32_t InstructionID;

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

class IDGenerator {
    private:
    std::mt19937 rng;
    std::uniform_int_distribution<uint32_t> distribution;

    public:
    IDGenerator(): rng(std::random_device{}()), distribution(std::uniform_int_distribution<uint32_t>(0, UINT32_MAX)) {}

    InstructionID new_instr_id();
};

#endif