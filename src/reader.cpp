
#include <cctype>
#include <vector>
#include "taxscan.h"

TokenKind kind(char c) {
    if (c == '`') {
        return WORD;
    }
    if (std::isalpha(c) || std::isdigit(c) || c == '_') {
        return WORD;
    }
    if (std::isspace(c)) {
        return WHITESPACE;
    }
    return SYMBOL;
}

Line parse(std::string value) {
    LineToken* current = nullptr;
    std::vector<LineToken> tokens;
    bool in_backquotes = false;
    for (int i = 0; i < value.length(); i++) {
        const char c = value[i];
        if (c == '`') {
            in_backquotes = !in_backquotes;
            continue;
        }
        const TokenKind char_kind = in_backquotes ? WORD : kind(c);

        if (current == nullptr || char_kind != current->kind) {
            tokens.push_back({.kind = char_kind, .value = std::string(1, c)});
            current = &tokens.back();
            continue;
        }

        if (char_kind == SYMBOL) {
            tokens.push_back({.kind = SYMBOL, .value = std::string(1, c)});
            current = nullptr;
            continue;
        }

        current->value += std::string(1, c);
    }

    int start = -1;
    int end = -1;
    int word_start = -1;
    for (size_t idx = 0; idx < tokens.size(); ++idx) {
        const LineToken token = tokens[idx];
        if (token.kind != WHITESPACE) {
            end = idx;
        }
        if (token.kind != WHITESPACE && start == -1) {
            start = idx;
        }
        if (token.kind == WORD && word_start == -1) {
            word_start = idx;
        }
    }
    return {.start = start, .end = end, .word_start = word_start, .tokens = tokens};
}
