#pragma once

#include <deque>
#include <sstream>
#include <iostream>
#include <optional>
#include "lexer.hpp"
#include "source.hpp"
#include "location.hpp"

class Expr {
public:
	Location loc;

	// AST locations always have length 0 (AST nodes can be multi-line)
	Expr(const Location& loc) : loc({ loc.line, loc.colStart, loc.colStart }) {}
	virtual ~Expr() {}
	virtual void print(std::ostream& os) const = 0;

	template <typename T>
	T* as() {
		return dynamic_cast<T*>(this);
	}

protected:
	static void try_print(std::ostream& os, Expr* expr) {
		if (expr) {
			expr->print(os);
		} else {
			os << "<error>";
		}
	}
};

class EIntLit : public Expr {
public:
	long long value;

	EIntLit(const Location& loc, long long value)
		: Expr(loc), value(value) {}

	void print(std::ostream& os) const override {
		os << value;
	}
};

class EBoolLit : public Expr {
public:
	bool value;

	EBoolLit(const Location& loc, bool value)
		: Expr(loc), value(value) {}

	void print(std::ostream& os) const override {
		os << (value ? "true" : "false");
	}
};

class EIdent : public Expr {
public:
	std::string value;

	EIdent(const Location& loc, std::string value)
		: Expr(loc), value(std::move(value)) {}

	void print(std::ostream& os) const override {
		os << value;
	}
};

class ELet : public Expr {
public:
	EIdent* ident;
	Expr* value;
	Expr* body;

	ELet(const Location& loc, EIdent* ident, Expr* value, Expr* body)
		: Expr(loc), ident(ident), value(value), body(body) {}

	void print(std::ostream& os) const override {
		os << "let ";
		try_print(os, ident);
		os << " = (";
		try_print(os, value);
		os << ") in (";
		try_print(os, body);
		os << ")";
	}
};

class EIf : public Expr {
public:
	Expr* test;
	Expr* body;
	Expr* elseBody;

	EIf(const Location& loc, Expr* test, Expr* body, Expr* elseBody)
		: Expr(loc), test(test), body(body), elseBody(elseBody) {}

	void print(std::ostream& os) const override {
		os << "if (";
		try_print(os, test);
		os << ") then (";
		try_print(os, body);
		os << ") else (";
		try_print(os, elseBody);
		os << ")";
	}
};

class EFun : public Expr {
public:
	EIdent* ident;
	Expr* body;

	EFun(const Location& loc, EIdent* ident, Expr* body)
		: Expr(loc), ident(ident), body(body) {}

	void print(std::ostream& os) const override {
		os << "fun ";
		try_print(os, ident);
		os << " -> ";
		try_print(os, body);
	}
};

class EFix : public Expr {
public:
	EIdent* ident;
	Expr* body;

	EFix(const Location& loc, EIdent* ident, Expr* body)
		: Expr(loc), ident(ident), body(body) {}

	void print(std::ostream& os) const override {
		os << "fix ";
		try_print(os, ident);
		os << " -> ";
		try_print(os, body);
	}
};

class EFunAp : public Expr {
public:
	Expr* fun;
	Expr* arg;

	EFunAp(const Location& loc, Expr* fun, Expr* arg)
		: Expr(loc), fun(fun), arg(arg) {}

	void print(std::ostream& os) const override {
		os << "(";
		try_print(os, fun);
		os << ") (";
		try_print(os, arg);
		os << ")";
	}
};

class EUnaryOp : public Expr {
public:
	Token op;
	Expr* right;

	EUnaryOp(const Location& loc, Token op, Expr* right)
		: Expr(loc), op(op), right(right) {}

	void print(std::ostream& os) const override {
		os << op << "(";
		try_print(os, right);
		os << ")";
	}
};

class EBinOp : public Expr {
public:
	Expr* left;
	Token op;
	Expr* right;

	EBinOp(const Location& loc, Expr* left, Token op, Expr* right)
		: Expr(loc), left(left), op(op), right(right) {}

	void print(std::ostream& os) const override {
		os << "(";
		try_print(os, left);
		os << ") " << op << " (";
		try_print(os, right);
		os << ")";
	}
};

class Parser {
public:
	Parser(const Source& source, std::deque<Token> tokens) : source(source), tokens(std::move(tokens)) {}

	Expr* parse() {
		Expr* expr = parse_expr();
		expect_token(TokenType::Eof, false);
		return expr;
	}
private:
	const Source& source;
	std::deque<Token> tokens;

	struct BindingPower {
		// for left-associative, left < right
		// for right-associative, right < left
		int left;
		int right;

		static BindingPower FunAp() {
			return {60, 61};
		}

		static BindingPower BinOp(const Token& op) {
			switch (op.type) {
			case TokenType::Mul:
			case TokenType::Div:
			case TokenType::Mod:
				return {50, 51};
				break;
			case TokenType::Plus:
			case TokenType::Minus:
				return {40, 41};
				break;
			case TokenType::Equals:
			case TokenType::NotEquals:
			case TokenType::Lt:
			case TokenType::Gt:
			case TokenType::Leq:
			case TokenType::Geq:
				return {30, 31};
				break;
			case TokenType::And:
				return {21, 20};
				break;
			case TokenType::Or:
				return {11, 10};
				break;
			default:
				throw std::runtime_error("Unrecognized infix operator: " + std::to_string((int)op.type));
			}
		}
	};

	// Helpers
	std::optional<Token> expect_token(TokenType tokenType, bool reportErrors = true) {
		if (tokens.empty()) {
			if (reportErrors) {
				std::ostringstream oss;
				oss << "expected token '" << tokenType << "' before eof";
				source.report_error_at_eof(oss.str());
			}
			return std::nullopt;
		} else {
			Token front = tokens.front();
			if (front.type != tokenType) {
				if (reportErrors) {
					std::ostringstream oss;
					oss << "expected token '" << tokenType << "'; got '" << front << "'";
					source.report_error(
					    front.loc.line,
					    front.loc.colStart,
					    front.loc.colEnd - front.loc.colStart,
					    oss.str());
				}
				return std::nullopt;
			} else {
				tokens.pop_front();
				return front;
			}
		}
	}

	// Non-terminals
	// <Expr>
	Expr* parse_expr(int minBindingPower = 0, bool reportErrors = true) {
		if (tokens.empty()) {
			if (reportErrors) {
				source.report_error_at_eof("unexpected end of token stream");
			}
			return nullptr;
		}
		Token peek = tokens.front();
		Expr* lhs = nullptr;
		if (peek.type == TokenType::IntLit) {
			// IntLit
			expect_token(TokenType::IntLit, reportErrors);
			long long value;
			try {
				value = std::stoll(peek.value);
			} catch (std::exception&) {
				if (reportErrors) {
					source.report_error(
					    peek.loc.line,
					    peek.loc.colStart,
					    peek.loc.colEnd - peek.loc.colStart,
					    "");
				}
				return nullptr;
			}
			lhs = new EIntLit(peek.loc, value);
		} else if (peek.type == TokenType::True) {
			// BoolLit(true)
			expect_token(TokenType::True, reportErrors);
			lhs = new EBoolLit(peek.loc, true);
		} else if (peek.type == TokenType::False) {
			// BoolLit(false)
			expect_token(TokenType::False, reportErrors);
			lhs = new EBoolLit(peek.loc, false);
		} else if (peek.type == TokenType::Ident) {
			// Ident
			expect_token(TokenType::Ident, reportErrors);
			lhs = new EIdent(peek.loc, peek.value);
		} else if (peek.type == TokenType::LeftParen) {
			// '(' <Expr> ')'
			expect_token(TokenType::LeftParen, reportErrors);
			lhs = parse_expr();
			expect_token(TokenType::RightParen, reportErrors);
		} else if (peek.type == TokenType::Let) {
			// <ELet>
			expect_token(TokenType::Let, reportErrors);
			std::optional<Token> identToken = expect_token(TokenType::Ident, reportErrors);
			if (!identToken) {
				return nullptr;
			}
			EIdent* ident = new EIdent(identToken->loc, identToken->value);
			expect_token(TokenType::Equals, reportErrors);
			Expr* value = parse_expr();
			expect_token(TokenType::In, reportErrors);
			Expr* body = parse_expr();
			lhs = new ELet(peek.loc, ident, value, body);
		} else if (peek.type == TokenType::If) {
			// <EIf>
			expect_token(TokenType::If, reportErrors);
			Expr* test = parse_expr();
			expect_token(TokenType::Then, reportErrors);
			Expr* body = parse_expr();
			expect_token(TokenType::Else, reportErrors);
			Expr* elseBody = parse_expr();
			lhs = new EIf(peek.loc, test, body, elseBody);
		} else if (peek.type == TokenType::Fun) {
			// <EFun>
			expect_token(TokenType::Fun, reportErrors);
			std::optional<Token> identToken = expect_token(TokenType::Ident, reportErrors);
			if (!identToken) {
				return nullptr;
			}
			EIdent* ident = new EIdent(identToken->loc, identToken->value);
			expect_token(TokenType::Arrow, reportErrors);
			Expr* body = parse_expr();
			lhs = new EFun(peek.loc, ident, body);
		} else if (peek.type == TokenType::Fix) {
			// <EFix>
			expect_token(TokenType::Fix, reportErrors);
			std::optional<Token> identToken = expect_token(TokenType::Ident, reportErrors);
			if (!identToken) {
				return nullptr;
			}
			EIdent* ident = new EIdent(identToken->loc, identToken->value);
			expect_token(TokenType::Arrow, reportErrors);
			Expr* body = parse_expr();
			lhs = new EFix(peek.loc, ident, body);
		} else if (peek.type == TokenType::Minus) {
			// '-' <Expr>
			expect_token(TokenType::Minus, reportErrors);
			Expr* right = parse_expr();
			lhs = new EUnaryOp(peek.loc, peek, right);
		} else if (peek.type == TokenType::Not) {
			// '!' <Expr>
			expect_token(TokenType::Not, reportErrors);
			Expr* right = parse_expr();
			lhs = new EUnaryOp(peek.loc, peek, right);
		} else {
			// unexpected token
			if (reportErrors) {
				std::ostringstream oss;
				oss << "unexpected token '" << peek.type << "'";
				source.report_error(
				    peek.loc.line,
				    peek.loc.colStart,
				    peek.loc.colEnd - peek.loc.colStart,
				    oss.str());
			}
			return nullptr;
		}
		// handle recursive cases with Pratt parsing
		// handle <EBinOp>
		while (true) {
			if (tokens.empty()) {
				break;
			}
			peek = tokens.front();
			bool matched = false;
			switch (peek.type) {
			case TokenType::Mul:
			case TokenType::Div:
			case TokenType::Mod:
			case TokenType::Plus:
			case TokenType::Minus:
			case TokenType::Equals:
			case TokenType::NotEquals:
			case TokenType::Lt:
			case TokenType::Gt:
			case TokenType::Leq:
			case TokenType::Geq:
			case TokenType::And:
			case TokenType::Or:
				BindingPower bindingPower = BindingPower::BinOp(peek);
				if (bindingPower.left < minBindingPower) {
					break;
				}
				matched = true;
				tokens.pop_front();
				Expr* rhs = parse_expr(bindingPower.right);
				if (!rhs) {
					break;
				}
				lhs = new EBinOp(lhs->loc, lhs, peek, rhs);
				break;
			}
			if (!matched) {
				break;
			}
		}
		// handle <EFunAp>
		while (true) {
			BindingPower bindingPower = BindingPower::FunAp();
			if (bindingPower.left < minBindingPower) {
				break;
			}
			// it's a little hacky to call parse_expr to check if we can create a EFunAp..
			// this trick relies on parse_expr not chomping tokens if it returns a nullptr
			Expr* rhs = parse_expr(bindingPower.right, false);
			if (!rhs) {
				break;
			}
			lhs = new EFunAp(lhs->loc, lhs, rhs);
		}
		return lhs;
	}
};

std::ostream& operator<<(std::ostream& os, const Expr* expr) {
	expr->print(os);
	return os;
}
