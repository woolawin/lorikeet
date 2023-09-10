#ifndef LK_TAXSCAN
#define LK_TAXSCAN

enum TokenKind {
    WHITESPACE,
    WORD,
    SYMBOL
};

struct LineToken {
    TokenKind kind;
    char value[];
};

struct Line {
    int start;
    int end;
    int word_start;
    LineToken tokens[];
};

Line parse(char value[]);

#endif