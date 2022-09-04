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

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		return Type::Unit();
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		return type_syn(typeCtx, false) == type;
	}

	void print_impl(std::ostream& os) const override {
		os << "()";
	}
};
