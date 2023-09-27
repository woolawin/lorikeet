#ifndef LK_CORE_TYPES
#define LK_CORE_TYPES

#include <iostream>
#include <string>
#include <vector>
#include <random>
#include "line.h"

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

struct BranchTaxonomy;

struct StatementTaxonomy {
    std::string name;
    std::vector<Line> input;
    std::vector<BranchTaxonomy> branches;

    bool operator==(const StatementTaxonomy& other) const;
    friend std::ostream& operator<<(std::ostream& os, const StatementTaxonomy& line);

    BranchTaxonomy& branch(bool is_default, const Line& line);
};

struct RoutineTaxonomy {
    std::vector<StatementTaxonomy> statements;

    bool operator==(const RoutineTaxonomy& other) const;
    friend std::ostream& operator<<(std::ostream& os, const RoutineTaxonomy& line);

    StatementTaxonomy& append(const Line& line);
};

struct BranchTaxonomy {
    bool default_branch;
    Line input;
    RoutineTaxonomy routine;

    bool operator==(const BranchTaxonomy& other) const;
    friend std::ostream& operator<<(std::ostream& os, const BranchTaxonomy& line);
};

class IDGenerator {
    public:
    virtual InstructionID new_instr_id() = 0;
};

class RandomIDGenerator: public IDGenerator {
    private:
    std::mt19937 rng;
    std::uniform_int_distribution<uint32_t> distribution;

    public:
    RandomIDGenerator():
        rng(std::random_device{}()),
        distribution(std::uniform_int_distribution<uint32_t>(0, UINT32_MAX)) {}

    InstructionID new_instr_id();
};

std::string indent(int count);
void to_stream(std::ostream& os, const RoutineTaxonomy& routine, int indentation);

#endif