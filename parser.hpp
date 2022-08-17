#pragma once

#include <deque>
#include <sstream>
#include <iostream>
#include <optional>
#include "expr.hpp"
#include "token.hpp"
#include "source.hpp"
#include "location.hpp"

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
			expect_token(TokenType::Equals, reportErrors);
			Expr* value = parse_expr();
			expect_token(TokenType::In, reportErrors);
			Expr* body = parse_expr();
			lhs = new ELet(peek.loc, identToken->value, value, body);
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
			expect_token(TokenType::Arrow, reportErrors);
			Expr* body = parse_expr();
			lhs = new EFun(peek.loc, identToken->value, body);
		} else if (peek.type == TokenType::Fix) {
			// <EFix>
			expect_token(TokenType::Fix, reportErrors);
			std::optional<Token> identToken = expect_token(TokenType::Ident, reportErrors);
			if (!identToken) {
				return nullptr;
			}
			expect_token(TokenType::Arrow, reportErrors);
			Expr* body = parse_expr();
			lhs = new EFix(peek.loc, identToken->value, body);
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
			case TokenType::Or: {
				// handle <EBinOp>
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
			default: {
				// handle <EFunAp>
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
				matched = true;
				lhs = new EFunAp(lhs->loc, lhs, rhs);
				break;
			}
			}
			if (!matched) {
				break;
			}
		}
		return lhs;
	}
};
