#pragma once

#include <deque>
#include <cctype>
#include <string>
#include <vector>
#include <cstring>
#include "source.hpp"
#include "location.hpp"

enum class TokenType {
	Error, Eof,
	Ident, IntLit,
	Let, In, If, Then, Else, Fun,
	Equals, NotEquals, Not,
	Lt, Gt, Leq, Geq,
	And, Or,
	Plus, Minus, Mul, Div, Mod,
	LeftParen, RightParen, Arrow
};

struct Token {
	Location loc;
	TokenType type;
	std::string value;
};

class Lexer {
public:
	Lexer(const Source& source) : source(source) {}

	std::deque<Token> get_tokens() {
		if (lexed) {
			throw std::runtime_error("Attempting to lex twice on same Lexer");
		}
		lexed = true;

		// generate token deque
		std::deque<Token> tokens;
		while (buf_valid()) {
			tokens.push_back(next());
		}
		int lastLine = (int)source.lines.size()-1;
		int lastLineCol = (int)source.lines[lastLine].size();
		tokens.push_back({ {lastLine, lastLineCol, 0}, TokenType::Eof, "" });

		// report errors for unterm comments
		for (const Location& loc : commentStack) {
			source.report_error(loc.line, loc.colStart, loc.colEnd - loc.colStart, "unterminated comment");
		}

		return tokens;
	}

private:
	const Source& source;
	bool lexed = false; // flag indicating whether lexing already occurred
	int line = 0;
	int col = 0;
	std::vector<Location> commentStack; // open comment locations

	Token next() {
		int colStart = col;
		if (try_consume("=")) {
			return { {line, colStart, col}, TokenType::Equals, "" };
		} else if (try_consume("!=")) {
			return { {line, colStart, col}, TokenType::NotEquals, "" };
		} else if (try_consume("!")) {
			return { {line, colStart, col}, TokenType::Not, "" };
		} else if (try_consume("<=")) {
			return { {line, colStart, col}, TokenType::Leq, "" };
		} else if (try_consume(">=")) {
			return { {line, colStart, col}, TokenType::Geq, "" };
		} else if (try_consume("<")) {
			return { {line, colStart, col}, TokenType::Lt, "" };
		} else if (try_consume(">")) {
			return { {line, colStart, col}, TokenType::Gt, "" };
		} else if (try_consume("->")) {
			return { {line, colStart, col}, TokenType::Arrow, "" };
		} else if (try_consume("&&")) {
			return { {line, colStart, col}, TokenType::And, "" };
		} else if (try_consume("||")) {
			return { {line, colStart, col}, TokenType::Or, "" };
		} else if (try_consume("+")) {
			return { {line, colStart, col}, TokenType::Plus, "" };
		} else if (try_consume("-")) {
			return { {line, colStart, col}, TokenType::Minus, "" };
		} else if (try_consume("*")) {
			return { {line, colStart, col}, TokenType::Mul, "" };
		} else if (try_consume("/")) {
			return { {line, colStart, col}, TokenType::Div, "" };
		} else if (try_consume("%")) {
			return { {line, colStart, col}, TokenType::Mod, "" };
		} else if (try_consume("(")) {
			return { {line, colStart, col}, TokenType::LeftParen, "" };
		} else if (try_consume(")")) {
			return { {line, colStart, col}, TokenType::RightParen, "" };
		} else if (try_consume("let")) {
			return { {line, colStart, col}, TokenType::Let, "" };
		} else if (try_consume("in")) {
			return { {line, colStart, col}, TokenType::In, "" };
		} else if (try_consume("if")) {
			return { {line, colStart, col}, TokenType::If, "" };
		} else if (try_consume("then")) {
			return { {line, colStart, col}, TokenType::Then, "" };
		} else if (try_consume("else")) {
			return { {line, colStart, col}, TokenType::Else, "" };
		} else if (try_consume("fun")) {
			return { {line, colStart, col}, TokenType::Fun, "" };
		} else {
			char c = get_char();
			if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_') {
				int colStart = col;
				while (col < get_line().size()) {
					++col;
					char c = get_char();
					if (!('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_' || '0' <= c && c <= '9' || c == '\'')) {
						int colEnd = col;
						return { {line, colStart, col}, TokenType::Ident, get_line().substr(colStart, colEnd - colStart) };
					}
				}
			} else if ('0' <= c && c <= '9') {
				int colStart = col;
				while (col < get_line().size()) {
					++col;
					char c = get_char();
					if (!('0' <= c && c <= '9')) {
						int colEnd = col;
						return { {line, colStart, col}, TokenType::IntLit, get_line().substr(colStart, colEnd - colStart) };
					}
				}
			}
		}
		// unrecognized char
		++col;
		std::string unrecognizedChar = get_line().substr(colStart, 1);
		source.report_error(line, colStart, col - colStart, "stray '" + unrecognizedChar + "' in program");
		return { {line, colStart, col}, TokenType::Error, unrecognizedChar };
	}

	// helpers
	const std::string& get_line() {
		return source.lines[line];
	}
	char get_char() {
		return get_line()[col];
	}

	bool buf_valid() {
		while (line < source.lines.size()) {
			if (col >= get_line().size()) {
				++line;
				col = 0;
				continue;
			}
			if (isspace(get_char())) {
				++col;
				continue;
			}
			if (try_consume("(*")) {
				commentStack.push_back({line, col-2, col});
				continue;
			}
			if (try_consume("*)")) {
				if (commentStack.empty()) {
					source.report_error(line, col-2, 2, "expected comment before '*)' token");
					continue;
				} else {
					commentStack.pop_back();
					continue;
				}
			}
			if (!commentStack.empty()) {
				++col;
				continue;
			}
			return true;
		}
		return false;
	}

	bool try_consume(const std::string& term) {
		if (col + term.size() > get_line().size()) {
			return false;
		} else {
			bool status = !strncmp(&get_line().c_str()[col], term.c_str(), term.size());
			if (status) {
				col += term.size();
				return true;
			} else {
				return false;
			}
		}
	}
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
		os << "num_lit";
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
	default:
		throw std::runtime_error("Unsupported TokenType: " + std::to_string((int)tokenType));
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
