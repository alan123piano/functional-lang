#pragma once

#include <deque>
#include <cctype>
#include <string>
#include <vector>
#include <cstring>
#include "token.hpp"
#include "source.hpp"
#include "location.hpp"

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
		} else if (try_consume("true")) {
			return { {line, colStart, col}, TokenType::True, "" };
		} else if (try_consume("false")) {
			return { {line, colStart, col}, TokenType::False, "" };
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
		} else if (try_consume("fix")) {
			return { {line, colStart, col}, TokenType::Fix, "" };
		} else if (try_consume("rec")) {
			return { {line, colStart, col}, TokenType::Rec, "" };
		} else {
			// check for identifier
			int size = peek_ident_size();
			if (size > 0) {
				int colStart = col;
				col += size;
				return { {line, colStart, col}, TokenType::Ident, get_line().substr(colStart, col - colStart) };
			}
			// check for int literal
			size = peek_int_lit_size();
			if (size > 0) {
				int colStart = col;
				col += size;
				return { {line, colStart, col}, TokenType::IntLit, get_line().substr(colStart, col - colStart) };
			}
		}
		// unrecognized char
		++col;
		std::string unrecognizedChar = get_line().substr(colStart, 1);
		source.report_error(line, colStart, col - colStart, "stray '" + unrecognizedChar + "' in program");
		return { {line, colStart, col}, TokenType::Error, unrecognizedChar };
	}

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
		// if we're consuming a keyword, it cannot also be ambiguously an
		// identifier (it must be on its own)
		// for example, fun_add is a single identifier, not 'fun' then '_add'
		if (col + term.size() > get_line().size()) {
			return false;
		} else {
			bool status = !strncmp(&get_line().c_str()[col], term.c_str(), term.size());
			if (status) {
				// block if an identifier is possible
				if (peek_ident_size() > term.size()) {
					return false;
				}
				col += term.size();
				return true;
			} else {
				return false;
			}
		}
	}

	// determines size for a hypothetical identifier
	int peek_ident_size() const {
		char c = get_char();
		int colEnd = col;
		if ('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_') {
			while (colEnd < get_line().size()) {
				++colEnd;
				char c = get_char(colEnd);
				if (!('a' <= c && c <= 'z' || 'A' <= c && c <= 'Z' || c == '_' || '0' <= c && c <= '9' || c == '\'')) {
					break;
				}
			}
		}
		return colEnd - col;
	}

	// determines size for a hypothetical int literal
	int peek_int_lit_size() const {
		char c = get_char();
		int colEnd = col;
		if ('0' <= c && c <= '9') {
			while (colEnd < get_line().size()) {
				++colEnd;
				char c = get_char(colEnd);
				if (!('0' <= c && c <= '9')) {
					break;
				}
			}
		}
		return colEnd - col;
	}
};
