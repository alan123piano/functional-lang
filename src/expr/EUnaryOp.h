#pragma once

#include "../Expr.h"

class EUnaryOp : public Expr {
public:
	Token op;
	Expr* right;

	EUnaryOp(const Location& loc, Token op, Expr* right)
		: Expr(loc), op(op), right(right) {}

	void print(std::ostream& os) const override {
		os << op;
		right->print(os);
	}

	Expr* copy() const override {
		return new EUnaryOp(loc, op, right->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newRight = right->subst(subIdent, subExpr);
		return new EUnaryOp(loc, op, newRight);
	}

	Value* eval() const override {
		Value* rightValue = right->eval();
		if (!rightValue) { return nullptr; }
		Value* result = OpDefinition::unary_op_result(op.type, rightValue);
		if (!result) {
			throw std::runtime_error("Attempted to evaluate ill-typed unary operation");
		}
		return result;
	}

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		// potential improvement: use (weaker) type analysis instead of synthesis?
		const Type* rtype = right->type_syn(typeCtx);
		if (!rtype) { return nullptr; }
		const Type* result = OpDefinition::unary_op_type(op.type, rtype);
		if (!result) {
			if (reportErrors) {
				std::ostringstream oss;
				oss << "expression (of " << rtype << " type) does not define unary operation " << op;
				right->report_error_at_expr(oss.str());
			}
			return nullptr;
		}
		return result;
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		return type_syn(typeCtx, false) == type;
	}
};
