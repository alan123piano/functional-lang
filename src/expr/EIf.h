#pragma once

#include "../Expr.h"

class EIf : public Expr {
public:
	Expr* test;
	Expr* body;
	Expr* elseBody;

	EIf(const Location& loc, const Type* typeAnn, Expr* test, Expr* body, Expr* elseBody)
		: Expr(loc, typeAnn), test(test), body(body), elseBody(elseBody) {}

	Expr* copy() const override {
		return new EIf(loc, typeAnn, test->copy(), body->copy(), elseBody->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newTest = test->subst(subIdent, subExpr);
		Expr* newBody = body->subst(subIdent, subExpr);
		Expr* newElseBody = elseBody->subst(subIdent, subExpr);
		return new EIf(loc, typeAnn, newTest, newBody, newElseBody);
	}

	Value* eval() const override {
		Value* testValue = test->eval();
		if (!testValue) {
			return nullptr;
		}
		VBool* cond = test->as<VBool>();
		if (!cond) {
			report_error_at_expr("expected expression of bool type in condition for if statement; got type " + testValue->type_name());
			return nullptr;
		}
		if (cond->value) {
			return body->eval();
		} else {
			return elseBody->eval();
		}
	}

	void print_impl(std::ostream& os) const override {
		os << "(if ";
		print(os, test);
		os << " then ";
		print(os, body);
		os << " else ";
		print(os, elseBody);
		os << ")";
	}
};
