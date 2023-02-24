#pragma once

#include "../Expr.h"
#include "EFun.h"

class EFunAp : public Expr {
public:
	Expr* fun;
	Expr* arg;

	EFunAp(const Location& loc, Expr* fun, Expr* arg)
		: Expr(loc), fun(fun), arg(arg) {}

	void print(std::ostream& os) const override {
		os << "(";
		fun->print(os);
		os << " ";
		arg->print(os);
		os << ")";
	}

	Expr* copy() const override {
		return new EFunAp(loc, fun->copy(), arg->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newFun = fun->subst(subIdent, subExpr);
		Expr* newArg = arg->subst(subIdent, subExpr);
		return new EFunAp(loc, newFun, newArg);
	}

	Value* eval() const override {
		Value* left = fun->eval();
		if (!left) { return nullptr; }
		const VFun* funValue = left->as<VFun>();
		if (!funValue) {
			throw std::runtime_error("Attempted to evaluate ill-typed function application");
		}
		const EFun* funExpr = funValue->fun->as<EFun>();
		if (!funExpr) {
			throw std::runtime_error("Failed to cast VFun fun to EFun");
		}
		Value* right = arg->eval();
		if (!right) { return nullptr; }
		return funExpr->body->subst(funExpr->var->ident, arg)->eval();
	}

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		const Type* funType = fun->type_syn(typeCtx);
		if (!funType) { return nullptr; }
		const TArrow* arrowType = funType->as<TArrow>();
		if (!arrowType) {
			if (reportErrors) {
				std::ostringstream oss;
				oss << "expected expression of arrow type in function application; got type " << funType;
				report_error_at_expr(oss.str());
			}
			return nullptr;
		}
		if (!arg->type_ana(arrowType->left, typeCtx)) {
			if (reportErrors) {
				std::ostringstream oss;
				oss << "expected expression of type " << arrowType->left << " as function argument";
				arg->report_error_at_expr(oss.str());
			}
			return nullptr;
		}
		return arrowType->right;
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		return type_syn(typeCtx, false) == type;
	}
};
