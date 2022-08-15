#pragma once

#include <deque>
#include <sstream>
#include <iostream>
#include <optional>
#include "file.hpp"
#include "lexer.hpp"
#include "location.hpp"

/**
 * <Expr> ::= <LetExpr>
 *          | <IfExpr>
 * 			| <FunExpr>
 *          | <FunAp>
 *          | <UnaryOp>
 *          | <BinOp>
 *          | IntLit
 * 			| Ident
 *          | '(' <Expr> ')'
 *
 * <LetExpr> ::= 'let' Ident '=' <Expr> 'in' <Expr>
 *
 * <IfExpr> ::= 'if' <Expr> 'then' <Expr> 'else' <Expr>
 *
 * <FunExpr> ::= 'fun' Ident '->' <Expr>
 *
 * <FunAp> ::= <Expr> <Expr>
 *
 * <UnaryOp> ::= '+' <Expr>
 * 			   | '-' <Expr>
 * 			   | '!' <Expr>
 *
 * <BinOp> ::= <Expr> '=' <Expr>
 * 			 | <Expr> '!=' <Expr>
 * 			 | <Expr> '<' <Expr>
 * 			 | <Expr> '>' <Expr>
 * 			 | <Expr> '<=' <Expr>
 * 			 | <Expr> '>=' <Expr>
 * 			 | <Expr> '&&' <Expr>
 * 			 | <Expr> '||' <Expr>
 * 			 | <Expr> '+' <Expr>
 * 			 | <Expr> '-' <Expr>
 * 			 | <Expr> '*' <Expr>
 * 			 | <Expr> '/' <Expr>
 * 			 | <Expr> '%' <Expr>
 *
 */

class Expr {
public:
	Expr() {}
	virtual ~Expr() {}
	virtual void print(std::ostream& os) = 0;

protected:
	void try_print(std::ostream& os, Expr* expr) {
		if (expr) {
			expr->print(os);
		} else {
			os << "<error>";
		}
	}
};

class IntLit : public Expr {
public:
	std::string value;

	IntLit(const std::string& value) : value(value) {}

	void print(std::ostream& os) override {
		os << value;
	}
};

class Ident : public Expr {
public:
	std::string value;

	Ident(const std::string& value) : value(value) {}

	void print(std::ostream& os) override {
		os << value;
	}
};

class LetExpr : public Expr {
public:
	Ident* ident;
	Expr* value;
	Expr* body;

	LetExpr(Ident* ident, Expr* value, Expr* body) : ident(ident), value(value), body(body) {}

	void print(std::ostream& os) override {
		os << "let ";
		try_print(os, ident);
		os << " = (";
		try_print(os, value);
		os << ") in (";
		try_print(os, body);
		os << ")";
	}
};

class IfExpr : public Expr {
public:
	Expr* test;
	Expr* body;
	Expr* elseBody;

	IfExpr(Expr* test, Expr* body, Expr* elseBody) : test(test), body(body), elseBody(elseBody) {}

	void print(std::ostream& os) override {
		os << "if (";
		try_print(os, test);
		os << ") then (";
		try_print(os, body);
		os << ") else (";
		try_print(os, elseBody);
		os << ")";
	}
};

class FunExpr : public Expr {
public:
	Ident* ident;
	Expr* body;

	FunExpr(Ident* ident, Expr* body) : ident(ident), body(body) {}

	void print(std::ostream& os) override {
		os << "fun ";
		try_print(os, ident);
		os << " -> ";
		try_print(os, body);
	}
};

class FunAp : public Expr {
public:
	Expr* fun;
	Expr* arg;

	FunAp(Expr* fun, Expr* arg) : fun(fun), arg(arg) {}

	void print(std::ostream& os) override {
		os << "(";
		try_print(os, fun);
		os << ") (";
		try_print(os, arg);
		os << ")";
	}
};

class UnaryOp : public Expr {
public:
	Token op;
	Expr* right;

	UnaryOp(Token op, Expr* right) : op(op), right(right) {}

	void print(std::ostream& os) override {
		os << op << "(";
		try_print(os, right);
		os << ")";
	}
};

class BinOp : public Expr {
public:
	Expr* left;
	Token op;
	Expr* right;

	BinOp(Expr* left, Token op, Expr* right) : left(left), op(op), right(right) {}

	void print(std::ostream& os) override {
		os << "(";
		try_print(os, left);
		os << ") " << op << " (";
		try_print(os, right);
		os << ")";
	}
};

class Parser {
public:
	Parser(const File& file, std::deque<Token> tokens) : file(file), tokens(std::move(tokens)) {}

	Expr* parse() {
		Expr* expr = parse_expr(0);
		expect_token(TokenType::Eof, false);
		return expr;
	}
private:
	const File& file;
	std::deque<Token> tokens;

	// Helpers
	std::optional<Token> expect_token(TokenType tokenType, bool reportError = true) {
		if (tokens.empty()) {
			if (reportError) {
				std::ostringstream oss;
				oss << "expected token '" << tokenType << "' before eof";
				file.report_error_at_eof(oss.str());
			}
			return std::nullopt;
		} else {
			Token front = tokens.front();
			if (front.type != tokenType) {
				if (reportError) {
					std::ostringstream oss;
					oss << "expected token '" << tokenType << "'; got '" << front << "'";
					file.report_error(
					    front.loc.line,
					    front.loc.colStart,
					    front.loc.colEnd - front.loc.colStart,
					    oss.str());
				}
				tokens.pop_front();
				return std::nullopt;
			} else {
				tokens.pop_front();
				return front;
			}
		}
	}

	// Non-terminals
	// <Expr>
	Expr* parse_expr(int bindingPower) {
		if (tokens.empty()) {
			file.report_error_at_eof("unexpected end of token stream");
			return nullptr;
		}
		Token peek = tokens.front();
		Expr* lhs;
		if (peek.type == TokenType::IntLit) {
			// IntLit
			std::optional<Token> intLitToken = expect_token(TokenType::IntLit);
			if (!intLitToken) {
				return nullptr;
			}
			lhs = new IntLit(intLitToken->value);
		} else if (peek.type == TokenType::Ident) {
			// Ident
			std::optional<Token> identToken = expect_token(TokenType::Ident);
			if (!identToken) {
				return nullptr;
			}
			lhs = new Ident(identToken->value);
		} else if (peek.type == TokenType::LeftParen) {
			// '(' <Expr> ')'
			expect_token(TokenType::LeftParen);
			lhs = parse_expr(0);
			expect_token(TokenType::RightParen);
		} else if (peek.type == TokenType::Let) {
			// <LetExpr>
			expect_token(TokenType::Let);
			std::optional<Token> identToken = expect_token(TokenType::Ident);
			if (!identToken) {
				return nullptr;
			}
			Ident* ident = new Ident(identToken->value);
			expect_token(TokenType::Equals);
			Expr* value = parse_expr(0);
			expect_token(TokenType::In);
			Expr* body = parse_expr(0);
			lhs = new LetExpr(ident, value, body);
		} else if (peek.type == TokenType::If) {
			// <IfExpr>
			expect_token(TokenType::If);
			Expr* test = parse_expr(0);
			expect_token(TokenType::Then);
			Expr* body = parse_expr(0);
			expect_token(TokenType::Else);
			Expr* elseBody = parse_expr(0);
			lhs = new IfExpr(test, body, elseBody);
		} else if (peek.type == TokenType::Fun) {
			// <FunExpr>
			expect_token(TokenType::Fun);
			std::optional<Token> identToken = expect_token(TokenType::Ident);
			if (!identToken) {
				return nullptr;
			}
			Ident* ident = new Ident(identToken->value);
			expect_token(TokenType::Arrow);
			Expr* body = parse_expr(0);
			lhs = new FunExpr(ident, body);
		} else if (peek.type == TokenType::Plus) {
			// '+' <Expr>
			std::optional<Token> op = expect_token(TokenType::Plus);
			if (!op) {
				return nullptr;
			}
			Expr* right = parse_expr(0);
			lhs = new UnaryOp(*op, right);
		} else if (peek.type == TokenType::Minus) {
			// '-' <Expr>
			std::optional<Token> op = expect_token(TokenType::Minus);
			if (!op) {
				return nullptr;
			}
			Expr* right = parse_expr(0);
			lhs = new UnaryOp(*op, right);
		} else if (peek.type == TokenType::Not) {
			// '!' <Expr>
			std::optional<Token> op = expect_token(TokenType::Not);
			if (!op) {
				return nullptr;
			}
			Expr* right = parse_expr(0);
			lhs = new UnaryOp(*op, right);
		} else {
			// unexpected token
			std::ostringstream oss;
			oss << "unexpected token '" << peek.type << "'";
			file.report_error(
			    peek.loc.line,
			    peek.loc.colStart,
			    peek.loc.colEnd - peek.loc.colStart,
			    oss.str());
			tokens.pop_front();
			return nullptr;
		}
		// handle recursive cases with Pratt parsing
		peek = tokens.front();
		// handle <BinOp>
		// handle <FunAp>
		return lhs;
	}
};
