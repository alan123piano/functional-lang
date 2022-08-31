#pragma once

#include <sstream>
#include "../Expr.h"
#include "../Type.h"
#include "../OpDefinition.h"
#include "../value/VBool.h"

class EBinaryOp : public Expr {
public:
	Expr* left;
	Token op;
	Expr* right;

	EBinaryOp(const Location& loc, const Type* typeAnn, Expr* left, Token op, Expr* right)
		: Expr(loc, typeAnn), left(left), op(op), right(right) {}

	Expr* copy() const override {
		return new EBinaryOp(loc, typeAnn, left->copy(), op, right->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newLeft = left->subst(subIdent, subExpr);
		Expr* newRight = right->subst(subIdent, subExpr);
		return new EBinaryOp(loc, typeAnn, newLeft, op, newRight);
	}

	Value* eval() const override {
		Value* leftValue = left->eval();
		if (!leftValue) { return nullptr; }
		Value* rightValue = right->eval();
		if (!rightValue) { return nullptr; }
		Value* result = OpDefinition::binary_op_result(leftValue, op.type, rightValue);
		if (!result) {
			throw std::runtime_error("Attempted to evaluate ill-typed binary operation");
		}
		return result;
	}

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		// potential improvement: use (weaker) type analysis instead of synthesis?
		const Type* ltype = left->type_syn(typeCtx);
		if (!ltype) { return nullptr; }
		const Type* rtype = right->type_syn(typeCtx);
		if (!rtype) { return nullptr; }
		const Type* result = OpDefinition::binary_op_type(ltype, op.type, rtype);
		if (!result) {
			if (reportErrors) {
				std::ostringstream oss;
				oss << "left expression (of " << ltype << " type) does not define operation "
				    << op << " with right expression (of " << rtype << " type)";
				left->report_error_at_expr(oss.str());
			}
			return nullptr;
		}
		return result;
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		return type_syn(typeCtx, false) == type;
	}

	void print_impl(std::ostream& os) const override {
		os << "(";
		print(os, left);
		os << " " << op << " ";
		print(os, right);
		os << ")";
	}
};
