#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "scope.hpp"
#include "value.hpp"
#include "parser.hpp"
#include "source.hpp"

class Evaluator {
public:
	Evaluator(const Source& source) : source(source) {}

	Value* eval(Expr* expr) {
		if (expr->as<EIntLit>()) {
			return new VInt(expr->as<EIntLit>()->value);
		} else if (expr->as<EBoolLit>()) {
			return new VBool(expr->as<EBoolLit>()->value);
		} else if (expr->as<EIdent>()) {
			EIdent* ident = expr->as<EIdent>();
			Value* value = scope.get(ident);
			if (!value) {
				source.report_error(
				    expr->loc.line,
				    expr->loc.colStart,
				    0,
				    "unbound variable '" + ident->value + "'");
				return nullptr;
			}
			return value;
		} else if (expr->as<ELet>()) {
			ELet* letExpr = expr->as<ELet>();
			EIdent* ident = letExpr->ident;
			scope.push(ident, eval(letExpr->value));
			Value* value = eval(letExpr->body);
			scope.pop(ident);
			return value;
		} else if (expr->as<EIf>()) {
			EIf* ifExpr = expr->as<EIf>();
			Value* test = eval(ifExpr->test);
			if (!test) {
				// don't report error here - we can assume that the failed call
				// to eval(ifExpr->test) already reported an error
				return nullptr;
			}
			VBool* cond = test->as<VBool>();
			if (!cond) {
				source.report_error(
				    ifExpr->test->loc.line,
				    ifExpr->test->loc.colStart,
				    0,
				    "expected expression of bool type in condition for if statement");
				return nullptr;
			}
			if (cond->value) {
				return eval(ifExpr->body);
			} else {
				return eval(ifExpr->elseBody);
			}
		} else if (expr->as<EFun>()) {
			return new VFun(expr->as<EFun>(), scope);
		} else if (expr->as<EFix>()) {
			// TODO
		} else if (expr->as<EFunAp>()) {
			EFunAp* funAp = expr->as<EFunAp>();
			Value* left = eval(funAp->fun);
			if (!left) {
				return nullptr;
			}
			VFun* fun = left->as<VFun>();
			if (!fun) {
				source.report_error(
				    funAp->fun->loc.line,
				    funAp->fun->loc.colStart,
				    0,
				    "expected expression of function type in function application");
				return nullptr;
			}
			Value* right = eval(funAp->arg);
			if (!right) {
				return nullptr;
			}
			Scope currScope = scope;
			scope = fun->scope;
			scope.push(fun->fun->ident, right);
			return eval(fun->fun->body);
			scope.pop(fun->fun->ident);
			scope = currScope;
		} else if (expr->as<EUnaryOp>()) {
			EUnaryOp* unaryOp = expr->as<EUnaryOp>();
			Value* right = eval(unaryOp->right);
			if (!right) {
				return nullptr;
			}
			switch (unaryOp->op.type) {
			case TokenType::Minus: {
				Value* result = right->negate();
				if (!result) {
					source.report_error(
					    unaryOp->right->loc.line,
					    unaryOp->right->loc.colStart,
					    0,
					    "expression (of " + right->type_name() + " type) does not define unary negation");
					return nullptr;
				}
				return result;
			}
			case TokenType::Not: {
				VBool* boolVal = right->as<VBool>();
				if (!boolVal) {
					source.report_error(
					    unaryOp->right->loc.line,
					    unaryOp->right->loc.colStart,
					    0,
					    "expected expression of boolean type in unary not");
					return nullptr;
				}
				return new VBool(!boolVal->value);
			}
			default:
				throw std::runtime_error("Unimplemented UnaryOp evaluation case");
			}
		} else if (expr->as<EBinOp>()) {
			EBinOp* binOp = expr->as<EBinOp>();
			Value* left = eval(binOp->left);
			if (!left) {
				return nullptr;
			}
			Value* right = eval(binOp->right);
			if (!right) {
				return nullptr;
			}
			switch (binOp->op.type) {
			case TokenType::Equals:
			case TokenType::NotEquals: {
				std::optional<bool> result = left->equals(right);
				if (!result) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "left expression (of " + left->type_name() + " type) does not define equality with right expression (of " + right->type_name() + " type)");
					return nullptr;
				}
				bool value = binOp->op.type == TokenType::Equals ? *result : !(*result);
				return new VBool(value);
			}
			case TokenType::Lt:
			case TokenType::Geq: {
				std::optional<bool> result = left->less_than(right);
				if (!result) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "left expression (of " + left->type_name() + " type) does not define less with right expression (of " + right->type_name() + " type)");
					return nullptr;
				}
				bool value = binOp->op.type == TokenType::Lt ? *result : !(*result);
				return new VBool(value);
			}
			case TokenType::Leq:
			case TokenType::Gt: {
				std::optional<bool> ltResult = left->less_than(right);
				if (!ltResult) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "left expression (of " + left->type_name() + " type) does not define less with right expression (of " + right->type_name() + " type)");
					return nullptr;
				}
				std::optional<bool> eqResult = left->equals(right);
				if (!eqResult) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "left expression (of " + left->type_name() + " type) does not define equality with right expression (of " + right->type_name() + " type)");
					return nullptr;
				}
				bool leq = *ltResult || *eqResult;
				bool value = binOp->op.type == TokenType::Leq ? leq : !leq;
				return new VBool(value);
			}
			case TokenType::And: {
				VBool* lbool = left->as<VBool>();
				if (!lbool) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "expected expression of boolean type in logical and");
					return nullptr;
				}
				VBool* rbool = right->as<VBool>();
				if (!rbool) {
					source.report_error(
					    binOp->right->loc.line,
					    binOp->right->loc.colStart,
					    0,
					    "expected expression of boolean type in logical and");
					return nullptr;
				}
				return new VBool(lbool->value && rbool->value);
			}
			case TokenType::Or: {
				VBool* lbool = left->as<VBool>();
				if (!lbool) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "expected expression of boolean type in logical or");
					return nullptr;
				}
				VBool* rbool = right->as<VBool>();
				if (!rbool) {
					source.report_error(
					    binOp->right->loc.line,
					    binOp->right->loc.colStart,
					    0,
					    "expected expression of boolean type in logical or");
					return nullptr;
				}
				return new VBool(lbool->value || rbool->value);
			}
			case TokenType::Plus: {
				Value* result = left->plus(right);
				if (!result) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "left expression (of " + left->type_name() + " type) does not define addition with right expression (of " + right->type_name() + " type)");
					return nullptr;
				}
				return result;
			}
			case TokenType::Minus: {
				Value* result = left->minus(right);
				if (!result) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "left expression (of " + left->type_name() + " type) does not define subtraction with right expression (of " + right->type_name() + " type)");
					return nullptr;
				}
				return result;
			}
			case TokenType::Mul: {
				Value* result = left->mul(right);
				if (!result) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "left expression (of " + left->type_name() + " type) does not define multiplication with right expression (of " + right->type_name() + " type)");
					return nullptr;
				}
				return result;
			}
			case TokenType::Div: {
				Value* result = left->div(right);
				if (!result) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "left expression (of " + left->type_name() + " type) does not define division with right expression (of " + right->type_name() + " type)");
					return nullptr;
				}
				return result;
			}
			case TokenType::Mod: {
				Value* result = left->mod(right);
				if (!result) {
					source.report_error(
					    binOp->left->loc.line,
					    binOp->left->loc.colStart,
					    0,
					    "left expression (of " + left->type_name() + " type) does not define modulo with right expression (of " + right->type_name() + " type)");
					return nullptr;
				}
				return result;
			}
			default:
				throw std::runtime_error("Unimplemented BinOp evaluation case");
			}
			return nullptr;
		}
		throw std::runtime_error("Unimplemented evaluation for expression");
	}

private:
	const Source& source;
	Scope scope;
};
