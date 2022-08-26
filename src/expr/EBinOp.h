#pragma once

#include "../Expr.h"
#include "../value/VBool.h"

class EBinOp : public Expr {
public:
	Expr* left;
	Token op;
	Expr* right;

	EBinOp(const Location& loc, const Type* typeAnn, Expr* left, Token op, Expr* right)
		: Expr(loc, typeAnn), left(left), op(op), right(right) {}

	Expr* copy() const override {
		return new EBinOp(loc, typeAnn, left->copy(), op, right->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newLeft = left->subst(subIdent, subExpr);
		Expr* newRight = right->subst(subIdent, subExpr);
		return new EBinOp(loc, typeAnn, newLeft, op, newRight);
	}

	Value* eval() const override {
		Value* leftValue = left->eval();
		if (!leftValue) { return nullptr; }
		Value* rightValue = right->eval();
		if (!rightValue) { return nullptr; }
		switch (op.type) {
		case TokenType::Equals:
		case TokenType::NotEquals: {
			std::optional<bool> result = leftValue->equals(rightValue);
			if (!result) {
				left->report_error_at_expr("left expression (of " + leftValue->type_name() + " type) does not define equality with right expression (of " + rightValue->type_name() + " type)");
				return nullptr;
			}
			bool value = op.type == TokenType::Equals ? *result : !(*result);
			return new VBool(value);
		}
		case TokenType::Lt:
		case TokenType::Geq: {
			std::optional<bool> result = leftValue->less_than(rightValue);
			if (!result) {
				left->report_error_at_expr("left expression (of " + leftValue->type_name() + " type) does not define less with right expression (of " + rightValue->type_name() + " type)");
				return nullptr;
			}
			bool value = op.type == TokenType::Lt ? *result : !(*result);
			return new VBool(value);
		}
		case TokenType::Leq:
		case TokenType::Gt: {
			std::optional<bool> ltResult = leftValue->less_than(rightValue);
			if (!ltResult) {
				left->report_error_at_expr("left expression (of " + leftValue->type_name() + " type) does not define less with right expression (of " + rightValue->type_name() + " type)");
				return nullptr;
			}
			std::optional<bool> eqResult = leftValue->equals(rightValue);
			if (!eqResult) {
				left->report_error_at_expr("left expression (of " + leftValue->type_name() + " type) does not define equality with right expression (of " + rightValue->type_name() + " type)");
				return nullptr;
			}
			bool leq = *ltResult || *eqResult;
			bool value = op.type == TokenType::Leq ? leq : !leq;
			return new VBool(value);
		}
		case TokenType::And: {
			VBool* lbool = leftValue->as<VBool>();
			if (!lbool) {
				left->report_error_at_expr("expected expression of boolean type in logical and; got type " + leftValue->type_name());
				return nullptr;
			}
			VBool* rbool = rightValue->as<VBool>();
			if (!rbool) {
				right->report_error_at_expr("expected expression of boolean type in logical and; got type " + rightValue->type_name());
				return nullptr;
			}
			return new VBool(lbool->value && rbool->value);
		}
		case TokenType::Or: {
			VBool* lbool = leftValue->as<VBool>();
			if (!lbool) {
				left->report_error_at_expr("expected expression of boolean type in logical or; got type " + leftValue->type_name());
				return nullptr;
			}
			VBool* rbool = rightValue->as<VBool>();
			if (!rbool) {
				right->report_error_at_expr("expected expression of boolean type in logical or; got type " + rightValue->type_name());
				return nullptr;
			}
			return new VBool(lbool->value || rbool->value);
		}
		case TokenType::Plus: {
			Value* result = leftValue->plus(rightValue);
			if (!result) {
				left->report_error_at_expr("left expression (of " + leftValue->type_name() + " type) does not define addition with right expression (of " + rightValue->type_name() + " type)");
				return nullptr;
			}
			return result;
		}
		case TokenType::Minus: {
			Value* result = leftValue->minus(rightValue);
			if (!result) {
				left->report_error_at_expr("left expression (of " + leftValue->type_name() + " type) does not define subtraction with right expression (of " + rightValue->type_name() + " type)");
				return nullptr;
			}
			return result;
		}
		case TokenType::Mul: {
			Value* result = leftValue->mul(rightValue);
			if (!result) {
				left->report_error_at_expr("left expression (of " + leftValue->type_name() + " type) does not define multiplication with right expression (of " + rightValue->type_name() + " type)");
				return nullptr;
			}
			return result;
		}
		case TokenType::Div: {
			Value* result = leftValue->div(rightValue);
			if (!result) {
				left->report_error_at_expr("left expression (of " + leftValue->type_name() + " type) does not define division with right expression (of " + rightValue->type_name() + " type)");
				return nullptr;
			}
			return result;
		}
		case TokenType::Mod: {
			Value* result = leftValue->mod(rightValue);
			if (!result) {
				left->report_error_at_expr("left expression (of " + leftValue->type_name() + " type) does not define modulo with right expression (of " + rightValue->type_name() + " type)");
				return nullptr;
			}
			return result;
		}
		default:
			throw std::runtime_error("Unimplemented BinOp evaluation case");
		}
		return nullptr;
	}

	void print_impl(std::ostream& os) const override {
		os << "(";
		print(os, left);
		os << " " << op << " ";
		print(os, right);
		os << ")";
	}
};
