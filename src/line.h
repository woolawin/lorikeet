#ifndef LK_LINE
#define LK_LINE

#include <string>
#include <vector>
#include <iostream>

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
    int line_num;
    int start;
    int end;
    int word_start;
    std::vector<LineToken> tokens;

    bool operator==(const Line& other) const;
    friend std::ostream& operator<<(std::ostream& os, const Line& line);

    std::string raw() const;
    std::string starting_whitespace() const;
    Line trim() const;
    const std::string& first_word() const;
    bool only_whitespace() const;
    bool empty() const;
    bool ends_with_symbol_seq(const std::string& symbol_seq) const;
    bool has_symbol_seq(size_t index, const std::string& symbol_seq) const;
    bool starts_with_symbol_seq(const std::string& symbol_seq) const;
    bool only_non_whitespace_equals(const std::string& value) const;
    bool is_seq_of_strings(const std::vector<std::string>& values) const;
    Line crop_from_first_word() const;
    void append(const Line& line);
    void append(char str);
};

std::vector<Line>& parse(const std::vector<std::string>& lines_raw, std::vector<Line>& lines);
Line parse(int line_num, std::string value);

Line quotize(Line& line);

enum class IndentationDiff {
    Increase,
    Decrease,
    Same,
    Error
};

struct Indentation {
    std::vector<std::string> indentations;

    IndentationDiff diff(const std::string& next_indentation) const;
    Indentation indent(const std::string& indentation) const;

};

#endif