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

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		const Type* type = typeCtx.get(value);
		if (!type) {
			if (reportErrors) {
				report_error_at_expr("unbound variable " + value);
			}
			return nullptr;
		} else {
			return type;
		}
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		return type_syn(typeCtx, false) == type;
	}

	void print_impl(std::ostream& os) const override {
		os << value;
	}
};
