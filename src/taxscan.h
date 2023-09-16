#ifndef LK_TAXSCAN
#define LK_TAXSCAN

#include <string>
#include <vector>
#include <iostream>
#include <memory>
#include <optional>

enum TokenKind {
    WHITESPACE,
    WORD,
    SYMBOL
};

struct LineToken {
    TokenKind kind;
    std::string value;

    bool operator==(const LineToken& other) const;
    friend std::ostream& operator<<(std::ostream& os, const LineToken& token);
};

struct Line {
    int start;
    int end;
    int word_start;
    std::vector<LineToken> tokens;

    bool operator==(const Line& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Line& line);

    std::string raw() const;
    const std::string& first_word() const;
    bool only_whitespace() const;
    bool ends_with_symbol_seq(const std::string& symbol_seq) const;
    bool has_symbol_seq(size_t index, const std::string& symbol_seq) const;
    bool starts_with_symbol_seq(const std::string& symbol_seq) const;
    bool only_non_whitespace_equals(const std::string& value) const;
    bool is_seq_of_strings(const std::vector<std::string>& values) const;
};

Line parse(std::string value);


struct BranchTaxonomy;

struct InstructionTaxonomy {
    std::string name;
    std::vector<Line> input;
    std::vector<BranchTaxonomy> branches;

    bool operator==(const InstructionTaxonomy& other) const;
    friend std::ostream& operator<<(std::ostream& os, const InstructionTaxonomy& line);

    void branch(bool is_default, const Line& line);
    BranchTaxonomy& current_branch();
};

struct RoutineTaxonomy {
    std::vector<InstructionTaxonomy> instructions;

    bool operator==(const RoutineTaxonomy& other) const;
    friend std::ostream& operator<<(std::ostream& os, const RoutineTaxonomy& line);

    InstructionTaxonomy& append(const Line& line);
    InstructionTaxonomy& current_instr();
};

struct BranchTaxonomy {
    bool defaultBranch; // @TODO rename to default_branch
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

class Peek {
    public:
	virtual const std::optional<Line> peek() = 0;
};

class Agent {
    public:
    virtual StepResult step(const InstructionTaxonomy& instr, Peek& peek) = 0;
    virtual SubroutineResult subroutine(const InstructionTaxonomy& instr, const Line& line) = 0;
};

FileTaxonomy scan_lines(const std::vector<std:: string>& lines, Agent& agent);
FileTaxonomy empty_file_taxonomy();

#endif