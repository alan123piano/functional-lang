#pragma once

#include <deque>
#include <limits>
#include <string>
#include <sstream>
#include <utility>
#include <iostream>
#include <optional>
#include <unordered_map>
#include <unordered_set>

#include "Expr.h"
#include "expr/EBinaryOp.h"
#include "expr/EBoolLit.h"
#include "expr/EFix.h"
#include "expr/EFloatLit.h"
#include "expr/EFun.h"
#include "expr/EFunAp.h"
#include "expr/EIf.h"
#include "expr/EIntLit.h"
#include "expr/ELet.h"
#include "expr/ERecordLit.h"
#include "expr/EUnaryOp.h"
#include "expr/EUnitLit.h"
#include "expr/EVar.h"

#include "Token.h"
#include "Source.h"

class Parser {
public:
	Parser(std::deque<Token> tokens) : tokens(std::move(tokens)) {}

	Expr* parse() {
		// parse and register type declarations
		while (tokens.front().type == TokenType::Type) {
			std::optional<std::pair<std::string, Type*>> typeDecl = parse_type_decl();
			if (!typeDecl) { break; }
			typeTable[typeDecl->first] = typeDecl->second;
		}
		// parse expression
		Expr* expr = parse_expr();
		if (expr) {
			expect_token(TokenType::Eof);
		}
		return expr;
	}

private:
	std::deque<Token> tokens;
	std::unordered_map<std::string, const Type*> typeTable = {
		{"int", Type::Int()},
		{"float", Type::Float()},
		{"bool", Type::Bool()},
		{"unit", Type::Unit()}
	};

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
	std::optional<Token> expect_token(TokenType tokenType) {
		if (tokens.empty()) {
			throw std::runtime_error("unexpected end of token buffer");
		} else {
			Token front = tokens.front();
			if (front.type != tokenType) {
				std::ostringstream oss;
				oss << "expected token '" << tokenType << "'; got '" << front << "'";
				front.report_error_at_token(oss.str());
				return std::nullopt;
			} else {
				tokens.pop_front();
				return front;
			}
		}
	}

	// Non-terminals
	// <Expr>
	Expr* parse_expr(int minBindingPower = 0, bool reportErrors = true);

	// <EVar>
	EVar* parse_ident();

	// <EType>
	const Type* parse_type_expr(bool reportErrors = true);

	// type [name] = [type]
	std::optional<std::pair<std::string, Type*>> parse_type_decl();
};
