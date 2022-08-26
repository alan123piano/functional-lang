#pragma once

#include "../Expr.h"
#include "EVar.h"
#include "../value/VFun.h"

class EFun : public Expr {
public:
	EVar* ident;
	Expr* body;

	EFun(const Location& loc, const Type* typeAnn, EVar* ident, Expr* body)
		: Expr(loc, typeAnn), ident(ident), body(body) {}

	Expr* copy() const override {
		return new EFun(loc, typeAnn, ident, body->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newBody;
		if (subIdent != ident->value) {
			newBody = body->subst(subIdent, subExpr);
		} else {
			newBody = body->copy();
		}
		return new EFun(loc, typeAnn, ident, newBody);
	}

	Value* eval() const override {
		return new VFun(this);
	}

	void print_impl(std::ostream& os) const override {
		os << "(fun ";
		print(os, ident);
		os << " -> ";
		print(os, body);
		os << ")";
	}
};
