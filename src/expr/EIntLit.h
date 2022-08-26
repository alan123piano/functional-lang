#pragma once

#include "../Expr.h"
#include "../value/VInt.h"

class EIntLit : public Expr {
public:
	int value;

	EIntLit(const Location& loc, const Type* typeAnn, int value)
		: Expr(loc, typeAnn), value(value) {}

	Expr* copy() const override {
		return new EIntLit(loc, typeAnn, value);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		return copy();
	}

	Value* eval() const override {
		return new VInt(value);
	}

	void print_impl(std::ostream& os) const override {
		os << value;
	}
};
