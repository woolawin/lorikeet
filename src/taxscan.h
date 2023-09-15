#ifndef LK_TAXSCAN
#define LK_TAXSCAN

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

    bool operator==(const LineToken& other) const {
        return this->kind == other.kind
            && this->value == other.value;
    }

    friend std::ostream& operator<<(std::ostream& os, const LineToken& token) {
        std::string kind_str;
        if (token.kind == WHITESPACE) {
            kind_str = "whitespace";
        } else if (token.kind == WORD) {
            kind_str = "word";
        } else {
            kind_str = "symbol";
        }
        os << "{kind: " << kind_str << ", value=`" << token.value << "`}";
        return os;
    }
};

struct Line {
    int start;
    int end;
    int word_start;
    std::vector<LineToken> tokens;

    bool operator==(const Line& other) const {
       return this->start == other.start
            && this->end == other.end
            && this->word_start == other.word_start
            && this->tokens == other.tokens;
    }

    friend std::ostream& operator<<(std::ostream& os, const Line& line) {
        os << "Line(\n\tstart=" << line.start << "\n\tend=" << line.end << "\n\tword_start=" << line.word_start;
        os << "\n\ttokens=[\n";
        for (const LineToken& token : line.tokens) {
            os << "\t\t" << token << "\n";
        }
        os << "\n\t]\n)";
        return os;
    }
};

Line parse(std::string value);

#endif