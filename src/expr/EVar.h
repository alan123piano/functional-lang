#pragma once

#include "../Expr.h"

class EVar : public Expr {
public:
	std::string ident;
	const Type* typeAnn;

	EVar(const Location& loc, std::string ident, const Type* typeAnn)
		: Expr(loc), ident(std::move(ident)), typeAnn(typeAnn) {}

	void print(std::ostream& os) const override {
		os << ident;
	}

	Expr* copy() const override {
		return new EVar(loc, ident, typeAnn);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		if (ident == subIdent) {
			return subExpr->copy();
		} else {
			return copy();
		}
	}

	Value* eval() const override {
		report_error_at_expr("unbound variable '" + ident + "'");
		return nullptr;
	}

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		const Type* type = typeCtx.get(ident);
		if (!type) {
			if (reportErrors) {
				report_error_at_expr("unbound variable " + ident);
			}
			return nullptr;
		} else {
			return type;
		}
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		return type_syn(typeCtx, false) == type;
	}
};
