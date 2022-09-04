#pragma once

#include "../Expr.h"
#include "../value/VBool.h"

class EBoolLit : public Expr {
public:
	bool value;

	EBoolLit(const Location& loc, const Type* typeAnn, bool value)
		: Expr(loc, typeAnn), value(value) {}

	Expr* copy() const override {
		return new EBoolLit(loc, typeAnn, value);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		return copy();
	}

	Value* eval() const override {
		return new VBool(value);
	}

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		return Type::Bool();
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		return type_syn(typeCtx, false) == type;
	}

	void print_impl(std::ostream& os) const override {
		os << (value ? "true" : "false");
	}
};
