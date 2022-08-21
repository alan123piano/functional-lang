#pragma once

#include <string>
#include <sstream>
#include "location.hpp"

enum class TokenType {
	Error, Eof,
	Ident, IntLit, True, False,
	Let, In, If, Then, Else,
	Fun, Fix, Rec,
	Equals, NotEquals, Not,
	Lt, Gt, Leq, Geq, And, Or,
	Plus, Minus, Mul, Div, Mod,
	LeftParen, RightParen,
	Arrow, Colon
};

struct Token {
	Location loc;
	TokenType type;
	std::string value;
};

std::ostream& operator<<(std::ostream& os, TokenType tokenType) {
	switch (tokenType) {
	case TokenType::Error:
		os << "error";
		break;
	case TokenType::Eof:
		os << "eof";
		break;
	case TokenType::Ident:
		os << "ident";
		break;
	case TokenType::IntLit:
		os << "int_lit";
		break;
	case TokenType::True:
		os << "true";
		break;
	case TokenType::False:
		os << "false";
		break;
	case TokenType::Let:
		os << "let";
		break;
	case TokenType::In:
		os << "in";
		break;
	case TokenType::If:
		os << "if";
		break;
	case TokenType::Then:
		os << "then";
		break;
	case TokenType::Else:
		os << "else";
		break;
	case TokenType::Fun:
		os << "fun";
		break;
	case TokenType::Fix:
		os << "fix";
		break;
	case TokenType::Rec:
		os << "rec";
		break;
	case TokenType::Equals:
		os << "=";
		break;
	case TokenType::NotEquals:
		os << "!=";
		break;
	case TokenType::Not:
		os << "!";
		break;
	case TokenType::Lt:
		os << "<";
		break;
	case TokenType::Gt:
		os << ">";
		break;
	case TokenType::Leq:
		os << "<=";
		break;
	case TokenType::Geq:
		os << ">=";
		break;
	case TokenType::And:
		os << "&&";
		break;
	case TokenType::Or:
		os << "||";
		break;
	case TokenType::Plus:
		os << "+";
		break;
	case TokenType::Minus:
		os << "-";
		break;
	case TokenType::Mul:
		os << "*";
		break;
	case TokenType::Div:
		os << "/";
		break;
	case TokenType::Mod:
		os << "%";
		break;
	case TokenType::LeftParen:
		os << "(";
		break;
	case TokenType::RightParen:
		os << ")";
		break;
	case TokenType::Arrow:
		os << "->";
		break;
	case TokenType::Colon:
		os << ":";
		break;
	}
	return os;
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
	os << token.type;
	if (token.value != "") {
		os << "(" << token.value << ")";
	}
	return os;
}
