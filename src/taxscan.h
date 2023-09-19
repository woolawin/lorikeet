#ifndef LK_TAXSCAN
#define LK_TAXSCAN

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <optional>
#include "line.h"

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

struct TaxScanError {
	std::string message;
	bool is_eof_error;

	bool operator==(const TaxScanError& other) const;
	friend std::ostream& operator<<(std::ostream& os, const TaxScanError& line);
};


struct FileTaxonomy {
	RoutineTaxonomy routine;
	std::optional<TaxScanError> err;

	bool operator==(const FileTaxonomy& other) const;
	friend std::ostream& operator<<(std::ostream& os, const FileTaxonomy& line);
};

struct BranchResult {
	bool is_default;
};

struct StepResult {
	std::optional<BranchResult> branch;
	size_t offset;
	size_t input_count;
	std::optional<TaxScanError> err;
};

struct SubroutineResult {
	bool add;
	bool step;
	std::optional<BranchResult> branch;
	size_t offset;
	std::optional<TaxScanError> err;
};

enum BlockKind {
    NA,
    INPUT,
    APPEND,
    ROUTINE
};

struct TaxStrat {
    BlockKind block_kind;
};

class Agent {
    public:
    virtual TaxStrat tax_strat(const std::string& instr_name) = 0;
};

FileTaxonomy scan_file(const std::vector<std:: string>& lines, Agent& agent);
FileTaxonomy empty_file_taxonomy();

#endif