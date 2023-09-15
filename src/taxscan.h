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
};

Line parse(std::string value);

#endif