#ifndef LK_TAXSCAN
#define LK_TAXSCAN

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <optional>

#include "line.h"
#include "errors.h"

struct BranchTaxonomy;

struct InstructionTaxonomy {
    std::string name;
    std::vector<Line> input;
    std::vector<BranchTaxonomy> branches;

    bool operator==(const InstructionTaxonomy& other) const;
    friend std::ostream& operator<<(std::ostream& os, const InstructionTaxonomy& line);

    BranchTaxonomy& branch(bool is_default, const Line& line);
};

struct RoutineTaxonomy {
    std::vector<InstructionTaxonomy> instructions;

    bool operator==(const RoutineTaxonomy& other) const;
    friend std::ostream& operator<<(std::ostream& os, const RoutineTaxonomy& line);

    InstructionTaxonomy& append(const Line& line);
};

struct BranchTaxonomy {
    bool default_branch;
    Line input;
    RoutineTaxonomy routine;

    bool operator==(const BranchTaxonomy& other) const;
    friend std::ostream& operator<<(std::ostream& os, const BranchTaxonomy& line);
};

struct FileTaxonomy {
	RoutineTaxonomy routine;
	std::vector<CompilationError> errors;

	bool operator==(const FileTaxonomy& other) const;
	friend std::ostream& operator<<(std::ostream& os, const FileTaxonomy& line);
};

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

class Agent {
    public:
    virtual TaxStrat tax_strat(const std::string& instr_name) = 0;
};

FileTaxonomy scan_file(const std::vector<std:: string>& lines, Agent& agent);
FileTaxonomy empty_file_taxonomy();

#endif