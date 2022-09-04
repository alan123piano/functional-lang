#include "Lexer.h"

std::deque<Token> Lexer::get_tokens() {
	if (lexed) {
		throw std::runtime_error("Attempting to lex twice on same Lexer");
	}
	lexed = true;

	// generate token deque
	std::deque<Token> tokens;
	while (buf_valid()) {
		tokens.push_back(next());
	}

	// push back eof
	int lastLine = (int)source.lines.size()-1;
	int lastLineCol = (int)source.lines[lastLine].size();
	tokens.push_back({ {&source, lastLine, lastLineCol, 0}, TokenType::Eof, "" });

	// report errors for unterm comments
	for (const Location& loc : commentStack) {
		source.report_error(loc.line, loc.colStart, loc.colEnd - loc.colStart, "unterminated comment");
	}

	return tokens;
}

Token Lexer::next() {
	int colStart = col;
	if (try_consume("!=")) {
		return { {&source, line, colStart, col}, TokenType::NotEquals, "" };
	} else if (try_consume("<=")) {
		return { {&source, line, colStart, col}, TokenType::Leq, "" };
	} else if (try_consume(">=")) {
		return { {&source, line, colStart, col}, TokenType::Geq, "" };
	} else if (try_consume("&&")) {
		return { {&source, line, colStart, col}, TokenType::And, "" };
	} else if (try_consume("||")) {
		return { {&source, line, colStart, col}, TokenType::Or, "" };
	} else if (try_consume("->")) {
		return { {&source, line, colStart, col}, TokenType::Arrow, "" };
	} else if (try_consume("=")) {
		return { {&source, line, colStart, col}, TokenType::Equals, "" };
	} else if (try_consume("!")) {
		return { {&source, line, colStart, col}, TokenType::Not, "" };
	} else if (try_consume("<")) {
		return { {&source, line, colStart, col}, TokenType::Lt, "" };
	} else if (try_consume(">")) {
		return { {&source, line, colStart, col}, TokenType::Gt, "" };
	} else if (try_consume("+")) {
		return { {&source, line, colStart, col}, TokenType::Plus, "" };
	} else if (try_consume("-")) {
		return { {&source, line, colStart, col}, TokenType::Minus, "" };
	} else if (try_consume("*")) {
		return { {&source, line, colStart, col}, TokenType::Mul, "" };
	} else if (try_consume("/")) {
		return { {&source, line, colStart, col}, TokenType::Div, "" };
	} else if (try_consume("%")) {
		return { {&source, line, colStart, col}, TokenType::Mod, "" };
	} else if (try_consume("(")) {
		return { {&source, line, colStart, col}, TokenType::LeftParen, "" };
	} else if (try_consume(")")) {
		return { {&source, line, colStart, col}, TokenType::RightParen, "" };
	} else if (try_consume(":")) {
		return { {&source, line, colStart, col}, TokenType::Colon, "" };
	} else if (try_consume("'")) {
		return { {&source, line, colStart, col}, TokenType::SingleQuote, "" };
	} else if (try_consume("|")) {
		return { {&source, line, colStart, col}, TokenType::Bar, "" };
	} else if (try_consume(",")) {
		return { {&source, line, colStart, col}, TokenType::Comma, "" };
	} else if (try_consume("{")) {
		return { {&source, line, colStart, col}, TokenType::LeftBrace, "" };
	} else if (try_consume("}")) {
		return { {&source, line, colStart, col}, TokenType::RightBrace, "" };
	} else if (try_consume(".")) {
		return { {&source, line, colStart, col}, TokenType::Dot, "" };
	} else if (try_consume(";")) {
		return { {&source, line, colStart, col}, TokenType::Semicolon, "" };
	} else if (try_consume("true")) {
		return { {&source, line, colStart, col}, TokenType::True, "" };
	} else if (try_consume("false")) {
		return { {&source, line, colStart, col}, TokenType::False, "" };
	} else if (try_consume("let")) {
		return { {&source, line, colStart, col}, TokenType::Let, "" };
	} else if (try_consume("in")) {
		return { {&source, line, colStart, col}, TokenType::In, "" };
	} else if (try_consume("if")) {
		return { {&source, line, colStart, col}, TokenType::If, "" };
	} else if (try_consume("then")) {
		return { {&source, line, colStart, col}, TokenType::Then, "" };
	} else if (try_consume("else")) {
		return { {&source, line, colStart, col}, TokenType::Else, "" };
	} else if (try_consume("fun")) {
		return { {&source, line, colStart, col}, TokenType::Fun, "" };
	} else if (try_consume("fix")) {
		return { {&source, line, colStart, col}, TokenType::Fix, "" };
	} else if (try_consume("rec")) {
		return { {&source, line, colStart, col}, TokenType::Rec, "" };
	} else if (try_consume("type")) {
		return { {&source, line, colStart, col}, TokenType::Type, "" };
	} else if (try_consume("match")) {
		return { {&source, line, colStart, col}, TokenType::Match, "" };
	} else if (try_consume("with")) {
		return { {&source, line, colStart, col}, TokenType::With, "" };
	} else {
		// check for identifier
		int size = peek_ident_size();
		if (size > 0) {
			int colStart = col;
			col += size;
			return { {&source, line, colStart, col}, TokenType::Ident, get_line().substr(colStart, col - colStart) };
		}
		// check for float literal
		size = peek_float_lit_size();
		if (size > 0) {
			int colStart = col;
			col += size;
			return { {&source, line, colStart, col}, TokenType::FloatLit, get_line().substr(colStart, col - colStart) };
		}
		// check for int literal
		size = peek_int_lit_size();
		if (size > 0) {
			int colStart = col;
			col += size;
			return { {&source, line, colStart, col}, TokenType::IntLit, get_line().substr(colStart, col - colStart) };
		}
	}
	// unrecognized char
	++col;
	std::string unrecognizedChar = get_line().substr(colStart, 1);
	source.report_error(line, colStart, col - colStart, "stray '" + unrecognizedChar + "' in program");
	return { {&source, line, colStart, col}, TokenType::Error, unrecognizedChar };
}

bool Lexer::buf_valid() {
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
			commentStack.push_back({&source, line, col-2, col});
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

bool Lexer::try_consume(const std::string& term) {
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

int Lexer::peek_ident_size() const {
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

int Lexer::peek_int_lit_size() const {
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

int Lexer::peek_float_lit_size() const {
	int colEnd = col;
	// [0-9]+
	while (colEnd < get_line().size()) {
		char c = get_char(colEnd);
		if (!('0' <= c && c <= '9')) {
			break;
		}
		++colEnd;
	}
	if (col == colEnd || colEnd == get_line().size()) { return 0; }
	// (.)
	if (get_char(colEnd) != '.') { return 0; }
	++colEnd;
	// [0-9]*
	while (colEnd < get_line().size()) {
		char c = get_char(colEnd);
		if (!('0' <= c && c <= '9')) {
			break;
		}
		++colEnd;
	}
	// ((E|e)(+|-)?[0-9]+)?
	if (colEnd < get_line().size() && (get_char(colEnd) == 'E' || get_char(colEnd) == 'e')) {
		++colEnd;
		if (colEnd < get_line().size() && (get_char(colEnd) == '+' || get_char(colEnd) == '-')) {
			++colEnd;
		}
		// [0-9]+
		int a = colEnd;
		while (colEnd < get_line().size()) {
			char c = get_char(colEnd);
			if (!('0' <= c && c <= '9')) {
				break;
			}
			++colEnd;
		}
		if (colEnd - a == 0) { return 0; }
	}
	return colEnd - col;
}
