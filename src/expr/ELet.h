#pragma once

#include <vector>
#include "../Expr.h"
#include "EVar.h"

class ELet : public Expr {
public:
	struct LetBody {

	};

	bool rec;
	EVar* var;
	Expr* value;
	Expr* body;

	ELet(const Location& loc, bool rec, EVar* var, Expr* value, Expr* body)
		: Expr(loc), rec(rec), var(var), value(value), body(body) {}

	void print(std::ostream& os) const override {
		os << "(let ";
		var->print(os);
		os << " = ";
		value->print(os);
		os << " in ";
		body->print(os);
		os << ")";
	}

	Expr* copy() const override {
		return new ELet(loc, rec, var, value->copy(), body->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newValue = value->subst(subIdent, subExpr);
		Expr* newBody;
		if (subIdent != var->ident) {
			newBody = body->subst(subIdent, subExpr);
		} else {
			newBody = body->copy();
		}
		return new ELet(loc, rec, var, newValue, newBody);
	}

	Value* eval() const override {
		return body->subst(var->ident, value)->eval();
	}

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		const Type* valueType = nullptr;
		if (var->typeAnn) {
			if (!value->type_ana(var->typeAnn, typeCtx)) {
				if (reportErrors) {
					std::ostringstream oss;
					oss << "expected expression of type " << var->typeAnn;
					value->report_error_at_expr(oss.str());
				}
				return nullptr;
			}
			valueType = var->typeAnn;
		} else {
			valueType = value->type_syn(typeCtx);
			if (!valueType) { return nullptr; }
		}
		Context<const Type*> ctx = typeCtx;
		ctx.push(var->ident, valueType);
		return body->type_syn(ctx);
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		const Type* valueType = nullptr;
		if (var->typeAnn) {
			if (!value->type_ana(var->typeAnn, typeCtx)) {
				return false;
			}
			valueType = var->typeAnn;
		} else {
			valueType = value->type_syn(typeCtx);
			if (!valueType) {
				return false;
			}
		}
		Context<const Type*> ctx = typeCtx;
		ctx.push(var->ident, valueType);
		return body->type_ana(type, ctx);
	}
};
