#pragma once

#include <deque>
#include <cctype>
#include <string>
#include <vector>
#include <cstring>
#include "Token.h"
#include "Source.h"

class Lexer {
public:
	Lexer(const Source& source) : source(source) {}

	std::deque<Token> get_tokens();

private:
	const Source& source;
	bool lexed = false; // flag indicating whether lexing already occurred
	int line = 0;
	int col = 0;
	std::vector<Location> commentStack; // open comment locations

	Token next();

	// helpers
	const std::string& get_line() const {
		return source.lines[line];
	}

	char get_char() const {
		return get_line()[col];
	}

	char get_char(int col) const {
		return get_line()[col];
	}

	bool buf_valid();

	bool try_consume(const std::string& term);

	// determines size for a hypothetical identifier
	int peek_ident_size() const;

	// determines size for a hypothetical int literal
	int peek_int_lit_size() const;

	// determines size for a hypothetical float literal
	// [0-9]+(.)[0-9]*((E|e)(+|-)?[0-9]+)?
	int peek_float_lit_size() const;
};
