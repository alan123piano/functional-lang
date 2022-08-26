#pragma once

#include "../Expr.h"
#include "EVar.h"

class ELet : public Expr {
public:
	EVar* ident;
	Expr* value;
	Expr* body;

	ELet(const Location& loc, const Type* typeAnn, EVar* ident, Expr* value, Expr* body)
		: Expr(loc, typeAnn), ident(ident), value(value), body(body) {}

	Expr* copy() const override {
		return new ELet(loc, typeAnn, ident, value->copy(), body->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newValue = value->subst(subIdent, subExpr);
		Expr* newBody;
		if (subIdent != ident->value) {
			newBody = body->subst(subIdent, subExpr);
		} else {
			newBody = body->copy();
		}
		return new ELet(loc, typeAnn, ident, newValue, newBody);
	}

	Value* eval() const override {
		return body->subst(ident->value, value)->eval();
	}

	void print_impl(std::ostream& os) const override {
		os << "(let ";
		print(os, ident);
		os << " = ";
		print(os, value);
		os << " in ";
		print(os, body);
		os << ")";
	}
};
