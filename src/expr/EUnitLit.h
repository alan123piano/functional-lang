#pragma once

#include "../Expr.h"
#include "../value/VUnit.h"

class EUnitLit : public Expr {
public:
	EUnitLit(const Location& loc, const Type* typeAnn)
		: Expr(loc, typeAnn) {}

	Expr* copy() const override {
		return new EUnitLit(loc, typeAnn);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		return copy();
	}

	Value* eval() const override {
		return new VUnit();
	}

	void print_impl(std::ostream& os) const override {
		os << "()";
	}
};
