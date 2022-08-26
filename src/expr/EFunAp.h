#pragma once

#include "../Expr.h"
#include "EFun.h"

class EFunAp : public Expr {
public:
	Expr* fun;
	Expr* arg;

	EFunAp(const Location& loc, const Type* typeAnn, Expr* fun, Expr* arg)
		: Expr(loc, typeAnn), fun(fun), arg(arg) {}

	Expr* copy() const override {
		return new EFunAp(loc, typeAnn, fun->copy(), arg->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newFun = fun->subst(subIdent, subExpr);
		Expr* newArg = arg->subst(subIdent, subExpr);
		return new EFunAp(loc, typeAnn, newFun, newArg);
	}

	Value* eval() const override {
		Value* left = fun->eval();
		if (!left) { return nullptr; }
		VFun* funValue = left->as<VFun>();
		if (!funValue) {
			report_error_at_expr("expected expression of function type in function application; got type " + left->type_name());
			return nullptr;
		}
		EFun* funExpr = fun->as<EFun>();
		Value* right = arg->eval();
		if (!right) { return nullptr; }
		return funExpr->body->subst(funExpr->ident->value, arg)->eval();
	}

	void print_impl(std::ostream& os) const override {
		os << "(";
		print(os, fun);
		os << " ";
		print(os, arg);
		os << ")";
	}
};
