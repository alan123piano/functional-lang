#pragma once

#include "../Expr.h"

class EVar : public Expr {
public:
	std::string value;

	EVar(const Location& loc, const Type* typeAnn, std::string value)
		: Expr(loc, typeAnn), value(std::move(value)) {}

	Expr* copy() const override {
		return new EVar(loc, typeAnn, value);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		if (value == subIdent) {
			return subExpr->copy();
		} else {
			return copy();
		}
	}

	Value* eval() const override {
		report_error_at_expr("unbound variable '" + value + "'");
		return nullptr;
	}

	void print_impl(std::ostream& os) const override {
		os << value;
	}
};
