#pragma once

#include "../Expr.h"
#include "EVar.h"

class EFix : public Expr {
public:
	EVar* ident;
	Expr* body;

	EFix(const Location& loc, const Type* typeAnn, EVar* ident, Expr* body)
		: Expr(loc, typeAnn), ident(ident), body(body) {}

	Expr* copy() const override {
		return new EFix(loc, typeAnn, ident, body->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newBody;
		if (subIdent != ident->value) {
			newBody = body->subst(subIdent, subExpr);
		} else {
			newBody = body->copy();
		}
		return new EFix(loc, typeAnn, ident, newBody);
	}

	Value* eval() const override {
		// evaluation currently uses substitution (quite expensive)
		return body->subst(ident->value, this)->eval();
	}

	void print_impl(std::ostream& os) const override {
		os << "(fix ";
		print(os, ident);
		os << " -> ";
		print(os, body);
		os << ")";
	}
};
