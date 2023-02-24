#pragma once

#include "../Expr.h"
#include "../value/VBool.h"

class EBoolLit : public Expr {
public:
	bool value;

	EBoolLit(const Location& loc, bool value)
		: Expr(loc), value(value) {}

	void print(std::ostream& os) const override {
		os << (value ? "true" : "false");
	}

	Expr* copy() const override {
		return new EBoolLit(loc, value);
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
};
