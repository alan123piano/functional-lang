#pragma once

#include "../Expr.h"

class EUnaryOp : public Expr {
public:
	Token op;
	Expr* right;

	EUnaryOp(const Location& loc, const Type* typeAnn, Token op, Expr* right)
		: Expr(loc, typeAnn), op(op), right(right) {}

	Expr* copy() const override {
		return new EUnaryOp(loc, typeAnn, op, right->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newRight = right->subst(subIdent, subExpr);
		return new EUnaryOp(loc, typeAnn, op, newRight);
	}

	Value* eval() const override {
		Value* rightValue = right->eval();
		if (!rightValue) { return nullptr; }
		switch (op.type) {
		case TokenType::Minus: {
			Value* result = rightValue->negate();
			if (!result) {
				right->report_error_at_expr("expression (of " + rightValue->type_name() + " type) does not define unary negation");
				return nullptr;
			}
			return result;
		}
		case TokenType::Not: {
			VBool* boolVal = right->as<VBool>();
			if (!boolVal) {
				right->report_error_at_expr("expected expression of boolean type in unary not; got type " + rightValue->type_name());
				return nullptr;
			}
			return new VBool(!boolVal->value);
		}
		default:
			throw std::runtime_error("Unimplemented UnaryOp evaluation case");
		}
	}

	void print_impl(std::ostream& os) const override {
		os << op;
		print(os, right);
	}
};
