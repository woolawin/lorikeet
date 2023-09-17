
#include <cctype>
#include <vector>
#include <sstream>
#include "line.h"

const std::string EMPTY_WORD = "";

bool LineToken::operator==(const LineToken& other) const {
    return this->kind == other.kind
        && this->value == other.value;
}

std::ostream& operator<<(std::ostream& os, const LineToken& token) {
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

bool Line::operator==(const Line& other) const {
   return this->start == other.start
        && this->end == other.end
        && this->word_start == other.word_start
        && this->tokens == other.tokens;
}

std::ostream& operator<<(std::ostream& os, const Line& line) {
    os << "Line(\n\tstart=" << line.start << "\n\tend=" << line.end << "\n\tword_start=" << line.word_start;
    os << "\n\ttokens=[\n";
    for (const LineToken& token : line.tokens) {
        os << "\t\t" << token << "\n";
    }
    os << "\n\t]\n)";
    return os;
}

const std::string& Line::first_word() const {
    if (this->word_start == -1) {
        return EMPTY_WORD;
    }
    return this->tokens[this->word_start].value;
}

bool Line::only_whitespace() const {
	return this->start == -1;
}

bool Line::ends_with_symbol_seq(const std::string& symbol_seq) const {
	if (this->end == -1) {
		return false;
	}
	if (this->end < symbol_seq.size() - 1) {
		return false;
	}
	const size_t length = symbol_seq.size();
	for (size_t idx = 0; idx < length; idx++) {
		const std::string c = std::string(1, symbol_seq[(length - 1) - idx]);
		if (c != this->tokens[this->end - idx].value) {
			return false;
		}
	}
	return true;
}

bool Line::has_symbol_seq(size_t index, const std::string& symbol_seq) const {
	if (this->start == -1) {
		return false;
	}
	if (this->tokens.size() < index + symbol_seq.size()) {
		return false;
	}
	for (size_t idx = 0; idx < symbol_seq.size(); idx++)  {
	    const std::string c = std::string(1, symbol_seq[idx]);
		if (c != this->tokens[index + idx].value) {
			return false;
		}
	}
	return true;
}

bool Line::starts_with_symbol_seq(const std::string& symbol_seq) const {
	return this->has_symbol_seq(this->start, symbol_seq);
}

bool Line::only_non_whitespace_equals(const std::string& value) const {
	if (this->start == -1 || this->start != this->end) {
		return false;
	}
	return this->tokens[this->start].value == value;
}

bool Line::is_seq_of_strings(const std::vector<std::string>& values) const {
	size_t non_whitespace = 0;
	for (size_t i = 0; i < this->tokens.size(); i++) {
		if (this->tokens[i].kind != WHITESPACE) {
			non_whitespace++;
		}
	}
	if (values.size() != non_whitespace) {
		return false;
	}
	size_t values_index = 0;
	for (size_t i = 0; i < this->tokens.size(); i++) {
		if (this->tokens[i].kind == WHITESPACE) {
			continue;
		}
		if (this->tokens[i].value != values[values_index]) {
			return false;
		}
		values_index++;
	}
	return values.size() == values_index;
}

std::string Line::raw() const {
    std::stringstream stream;
	for (size_t idx = 0; idx < this->tokens.size(); idx++)  {
		stream << this->tokens[idx].value;
	}
	return stream.str();
}

Line Line::crop_from_first_word() const {
    bool found_first_word = false;
    int crop_offset = 0;
    int start = -1;
    int end = -1;
    int word_start = -1;
    std::vector<LineToken> new_tokens;
    for (size_t idx = 0; idx < tokens.size(); idx++) {
        const LineToken token = tokens[idx];
        if (!found_first_word && token.kind != WORD) {
            continue;
        }
        if (!found_first_word && token.kind == WORD) {
            found_first_word = true;
            crop_offset = idx + 1;
            continue;
        }
        new_tokens.push_back(token);
        if (token.kind != WHITESPACE) {
            end = idx - crop_offset;
        }
        if (token.kind != WHITESPACE && start == -1) {
            start = idx - crop_offset;
        }
        if (token.kind == WORD && word_start == -1) {
            word_start = idx - crop_offset;
        }
    }
    return {.start = start, .end = end, .word_start = word_start, .tokens = new_tokens};
}

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
    for (size_t i = 0; i < value.length(); i++) {
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
    for (size_t idx = 0; idx < tokens.size(); idx++) {
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


