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

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		const Type* argType = ident->typeAnn;
		if (!argType) {
			if (reportErrors) {
				// TODO: when implementing generics, have more clever type inference systems..
				// ex. fun x -> x should synthesize as 'a -> 'a
				report_error_at_expr("type annotation is necessary to make function well-typed");
			}
			return nullptr;
		}
		Context<const Type*> ctx = typeCtx;
		ctx.push(ident->value, argType);
		return new TArrow(argType, body->type_syn(ctx));
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		if (type_syn(typeCtx, false) == type) { return true; }
		const TArrow* arrowType = type->as<TArrow>();
		if (!arrowType) { return false; }
		Context<const Type*> ctx = typeCtx;
		ctx.push(ident->value, arrowType->left);
		return body->type_ana(arrowType->right, ctx);
	}

	void print_impl(std::ostream& os) const override {
		os << "(fun ";
		print(os, ident);
		os << " -> ";
		print(os, body);
		os << ")";
	}
};
