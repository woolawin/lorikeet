#ifndef LK_TAXSCAN
#define LK_TAXSCAN

#include <string>
#include <vector>

enum TokenKind {
    WHITESPACE,
    WORD,
    SYMBOL
};

struct LineToken {
    TokenKind kind;
    std::string value;
};

struct Line {
    int start;
    int end;
    int word_start;
    std::vector<LineToken> tokens;
};

Line parse(std::string value);

#endif