#pragma once

#include <string>
#include <sstream>
#include "Source.h"

enum class TokenType {
	Error, Eof,
	Ident, IntLit, FloatLit, True, False,
	Let, In, If, Then, Else,
	Fun, Rec, Type, Match, With,
	Equals, NotEquals, Not,
	Lt, Gt, Leq, Geq, And, Or,
	Plus, Minus, Mul, Div, Mod,
	PlusDot, MinusDot, MulDot, DivDot,
	LeftParen, RightParen,
	Arrow, Colon, SingleQuote,
	Bar, Comma,
	LeftBrace, RightBrace, Dot,
	Semicolon
};

struct Token {
	Location loc;
	TokenType type;
	std::string value;

	void report_error_at_token(std::string error) const {
		loc.source->report_error(loc.line, loc.colStart, 0, std::move(error));
	}
};

std::ostream& operator<<(std::ostream& os, TokenType tokenType);
std::ostream& operator<<(std::ostream& os, const Token& token);
