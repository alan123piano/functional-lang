#pragma once

#include "../Expr.h"
#include "../value/VFloat.h"

class EFloatLit : public Expr {
public:
	long long value;

	EFloatLit(const Location& loc, const Type* typeAnn, long long value)
		: Expr(loc, typeAnn), value(value) {}

	Expr* copy() const override {
		return new EFloatLit(loc, typeAnn, value);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		return copy();
	}

	Value* eval() const override {
		return new VFloat(value);
	}

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		return Type::Float();
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		return type_syn(typeCtx, false) == type;
	}

	void print_impl(std::ostream& os) const override {
		os << value;
	}
};
