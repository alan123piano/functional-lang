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

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		const Type* valueType = nullptr;
		if (ident->typeAnn) {
			if (!value->type_ana(ident->typeAnn, typeCtx)) {
				if (reportErrors) {
					std::ostringstream oss;
					oss << "expected expression of type " << ident->typeAnn;
					value->report_error_at_expr(oss.str());
				}
				return nullptr;
			}
			valueType = ident->typeAnn;
		} else {
			valueType = value->type_syn(typeCtx);
			if (!valueType) { return nullptr; }
		}
		Context<const Type*> ctx = typeCtx;
		ctx.push(ident->value, valueType);
		return body->type_syn(ctx);
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		const Type* valueType = nullptr;
		if (ident->typeAnn) {
			if (!value->type_ana(ident->typeAnn, typeCtx)) {
				return false;
			}
			valueType = ident->typeAnn;
		} else {
			valueType = value->type_syn(typeCtx);
			if (!valueType) {
				return false;
			}
		}
		Context<const Type*> ctx = typeCtx;
		ctx.push(ident->value, valueType);
		return body->type_ana(type, ctx);
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
