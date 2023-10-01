
#include <cctype>
#include <vector>
#include <sstream>
#include "line.h"

const std::string EMPTY_WORD = "";

bool LineToken::operator==(const LineToken& other) const {
    return this->kind == other.kind
        && this->value == other.value
        && this->quote_mark == other.quote_mark
        && this->flag_prefix == other.flag_prefix;
}

std::ostream& operator<<(std::ostream& os, const LineToken& token) {
    std::string kind_str;
    if (token.kind == TokenKind::Whitespace) {
        kind_str = "whitespace";
    } else if (token.kind == TokenKind::Word) {
        kind_str = "word";
    } else if (token.kind == TokenKind::Quote) {
        kind_str = "quote";
    } else if (token.kind == TokenKind::Flag) {
        kind_str = "flag";
    } else {
        kind_str = "symbol";
    }
    os << "{kind: " << kind_str << ", value=`" << token.value << "` ";
    os << "quote_mark=`" << token.quote_mark << "` ";
    os << "flag_prefix=`" << token.flag_prefix <<"` ";
    os << "}";
    return os;
}

bool Line::operator==(const Line& other) const {
   return this->start == other.start
        && this->end == other.end
        && this->word_start == other.word_start
        && this->line_num == other.line_num
        && this->tokens == other.tokens;
}

std::ostream& operator<<(std::ostream& os, const Line& line) {
    os << "Line(\n\tstart=" << line.start << "\n\tend=" << line.end << "\n\tword_start=" << line.word_start;
    os << "\n\tline_num=" << line.line_num << "\n\ttokens=[\n";
    for (const LineToken& token : line.tokens) {
        os << "\t\t" << token << "\n";
    }
    os << "\n\t]\n)";
    return os;
}

LineToken generic_token(TokenKind kind, std::string value) {
    return { .kind = kind, .value = value, .quote_mark = "", .flag_prefix = "" };
}

LineToken word_token(std::string value) {
    return { .kind = TokenKind::Word, .value = value, .quote_mark = "", .flag_prefix = "" };
}

LineToken symbol_token(std::string value) {
    return { .kind = TokenKind::Symbol, .value = value, .quote_mark = "", .flag_prefix = "" };
}

LineToken whitespace_token(std::string value) {
    return { .kind = TokenKind::Whitespace, .value = value, .quote_mark = "", .flag_prefix = "" };
}

LineToken quote_token(std::string value, std::string quote_mark) {
    return { .kind = TokenKind::Quote, .value = value, .quote_mark = quote_mark, .flag_prefix = "" };
}

LineToken flag_token(std::string value, std::string prefix) {
    return { .kind = TokenKind::Flag, .value = value, .quote_mark = "", .flag_prefix = prefix };
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
		if (this->tokens[i].kind != TokenKind::Whitespace) {
			non_whitespace++;
		}
	}
	if (values.size() != non_whitespace) {
		return false;
	}
	size_t values_index = 0;
	for (size_t i = 0; i < this->tokens.size(); i++) {
		if (this->tokens[i].kind == TokenKind::Whitespace) {
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
        if (!found_first_word && token.kind != TokenKind::Word) {
            continue;
        }
        if (!found_first_word && token.kind == TokenKind::Word) {
            found_first_word = true;
            crop_offset = idx + 1;
            continue;
        }
        new_tokens.push_back(token);
        if (token.kind != TokenKind::Whitespace) {
            end = idx - crop_offset;
        }
        if (token.kind != TokenKind::Whitespace && start == -1) {
            start = idx - crop_offset;
        }
        if (token.kind == TokenKind::Word && word_start == -1) {
            word_start = idx - crop_offset;
        }
    }
    return {
        .line_num = this->line_num,
        .start = start,
        .end = end,
        .word_start = word_start,
        .tokens = new_tokens
    };
}

bool Line::empty() const {
    return this->tokens.empty();
}

std::string Line::starting_whitespace() const {
    if (this->start == 0) {
        return "";
    }
    return this->tokens[0].value;
}

void calculate_start_and_stops(Line& line) {
    for (size_t idx = 0; idx < line.tokens.size(); idx++) {
        const LineToken token = line.tokens[idx];
        if (token.kind != TokenKind::Whitespace) {
            line.end = idx;
        }
        if (token.kind != TokenKind::Whitespace && line.start == -1) {
            line.start = idx;
        }
        if (token.kind == TokenKind::Word && line.word_start == -1) {
            line.word_start = idx;
        }
    }
}

Line Line::trim() const {
    Line line = { .line_num = this->line_num, .start = -1, .end = -1, .word_start = -1, .tokens = {} };
    const size_t start = this->tokens[0].kind == TokenKind::Whitespace
        ? 1
        : 0;
    const size_t end = this->tokens.back().kind == TokenKind::Whitespace
        ? this->tokens.size() - 1
        : this->tokens.size();
    for (size_t idx = start; idx < end; idx++) {
        line.tokens.push_back(this->tokens[idx]);
    }
    calculate_start_and_stops(line);
    return line;
}

void Line::append(const Line& line) {
    for (LineToken token : line.tokens) {
        this->tokens.push_back(token);
    }
    calculate_start_and_stops(*this);
}

TokenKind kind(char c) {
    if (c == '`') {
        return TokenKind::Word;
    }
    if (std::isalpha(c) || std::isdigit(c) || c == '_') {
        return TokenKind::Word;
    }
    if (std::isspace(c)) {
        return TokenKind::Whitespace;
    }
    return TokenKind::Symbol;
}

void Line::append(char value) {
    this->tokens.push_back(generic_token(kind(value), std::string(1, value)));
    calculate_start_and_stops(*this);
}

Line parse(int line_num, std::string value) {
    LineToken* current = nullptr;
    std::vector<LineToken> tokens;
    bool in_backquotes = false;
    for (size_t i = 0; i < value.length(); i++) {
        const char c = value[i];
        if (c == '`') {
            in_backquotes = !in_backquotes;
            continue;
        }
        const TokenKind char_kind = in_backquotes ? TokenKind::Word : kind(c);

        if (current == nullptr || char_kind != current->kind) {
            tokens.push_back(generic_token(char_kind, std::string(1, c)));
            current = &tokens.back();
            continue;
        }

        if (char_kind == TokenKind::Symbol) {
            tokens.push_back(symbol_token(std::string(1, c)));
            current = nullptr;
            continue;
        }

        current->value += std::string(1, c);
    }
    Line line = { .line_num = line_num, .start = -1, .end = -1, .word_start = -1, .tokens = tokens };
    calculate_start_and_stops(line);
    return line;
}

std::vector<Line>& parse(const std::vector<std::string>& lines_raw, std::vector<Line>& lines) {
    for(int i = 0; i < lines_raw.size(); i++) {
        lines.push_back(parse(i + 1, lines_raw[i]));
    }
    return lines;
}


Line parse_quotes(const Line& line) {
    std::vector<LineToken> tokens = {};
    std::string quote = "";
    bool in_quotes;
    char quote_char = 0;
    for (size_t idx = 0; idx < line.tokens.size(); idx++) {
        const LineToken token = line.tokens[idx];
        if (token.kind != TokenKind::Symbol && in_quotes) {
            quote += token.value;
            continue;
        }

        if (token.kind != TokenKind::Symbol) {
            tokens.push_back(token);
            continue;
        }

        if (token.value == "\\") {
            LineToken next_token = line.tokens[idx + 1];
            if (next_token.kind == TokenKind::Symbol && next_token.value[0] == quote_char) {
                quote += quote_char;
                idx++;
                continue;
            }
        }

        if (token.value != "\"" && token.value != "'" && in_quotes) {
            quote += token.value;
            continue;
        }

        if (token.value != "\"" && token.value != "'") {
            tokens.push_back(token);
            continue;
        }

        if (in_quotes) {
            tokens.push_back(quote_token(quote, std::string(1, quote_char)));
            quote_char = 0;
            in_quotes = false;
            quote = "";
            continue;
        }

        in_quotes = true;
        quote_char = token.value[0];
    }
    Line quoted = { .line_num = line.line_num, .start = -1, .end = -1, .word_start = -1, .tokens = tokens };
    calculate_start_and_stops(quoted);
    return quoted;
}

Line parse_flags(const Line& line) {
    std::vector<LineToken> tokens;
    std::string flag = "";
    std::string flag_prefix = "";
    bool in_flag = false;
    bool found_first_flag_word = false;
    for (size_t idx = 0; idx < line.tokens.size(); idx++) {
        const LineToken token = line.tokens[idx];
        if (!in_flag && token.kind != TokenKind::Symbol && token.value != "-") {
            tokens.push_back(token);
            continue;
        }

        if (!in_flag && token.kind == TokenKind::Symbol && token.value == "-") {
            in_flag = true;
            flag_prefix = token.value;
            continue;
        }

        if (in_flag && token.kind == TokenKind::Whitespace) {
            if (!found_first_flag_word) {
                for (char c : flag_prefix) {
                    tokens.push_back(symbol_token(std::string(1, c)));
                }
                tokens.push_back(token);
            } else {
                tokens.push_back(flag_token(flag, flag_prefix));
                tokens.push_back(token);
            }
            in_flag = false;
            flag = "";
            flag_prefix = "";
            found_first_flag_word = false;
            continue;
        }

        if (in_flag && token.kind == TokenKind::Symbol && token.value == "-") {
            if (found_first_flag_word) {
                flag += token.value;
            } else {
                flag_prefix += token.value;
            }
            continue;
        }

        if (in_flag && token.kind == TokenKind::Word) {
            flag += token.value;
            found_first_flag_word = true;
            continue;
        }
    }
    Line flagged = { .line_num = line.line_num, .start = -1, .end = -1, .word_start = -1, .tokens = tokens };
    calculate_start_and_stops(flagged);
    return flagged;
}

IndentationDiff Indentation::diff(const std::string& next_indentation) const {
    if (this->indentations.empty()) {
        return next_indentation.size() == 0
            ? IndentationDiff::Same
            : IndentationDiff::Increase;
    }
    if (next_indentation == this->indentations.back()) {
        return IndentationDiff::Same;
    }
    for (size_t idx = 0; idx < this->indentations.size(); idx++) {
        if (this->indentations[idx] == next_indentation) {
            return IndentationDiff::Decrease;
        }
    }
    if (next_indentation.rfind(this->indentations.back(), 0) != 0) {
        return IndentationDiff::Error;
    }
    return IndentationDiff::Increase;
}

Indentation Indentation::indent(const std::string& indentation) const {
    std::vector<std::string> indentations = this->indentations;
    indentations.push_back(indentation);
    return { .indentations = indentations };
}