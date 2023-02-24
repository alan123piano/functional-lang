#pragma once

#include "../Expr.h"

class EIf : public Expr {
public:
	Expr* test;
	Expr* body;
	Expr* elseBody;

	EIf(const Location& loc, Expr* test, Expr* body, Expr* elseBody)
		: Expr(loc), test(test), body(body), elseBody(elseBody) {}

	void print(std::ostream& os) const override {
		os << "(if ";
		test->print(os);
		os << " then ";
		body->print(os);
		os << " else ";
		elseBody->print(os);
		os << ")";
	}

	Expr* copy() const override {
		return new EIf(loc, test->copy(), body->copy(), elseBody->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newTest = test->subst(subIdent, subExpr);
		Expr* newBody = body->subst(subIdent, subExpr);
		Expr* newElseBody = elseBody->subst(subIdent, subExpr);
		return new EIf(loc, newTest, newBody, newElseBody);
	}

	Value* eval() const override {
		Value* testValue = test->eval();
		if (!testValue) {
			return nullptr;
		}
		const VBool* cond = testValue->as<VBool>();
		if (!cond) {
			std::ostringstream oss;
			oss << "expected expression of bool type in condition for if statement; got type " << testValue->get_type();
			report_error_at_expr(oss.str());
			return nullptr;
		}
		if (cond->value) {
			return body->eval();
		} else {
			return elseBody->eval();
		}
	}

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		if (!test->type_ana(Type::Bool(), typeCtx)) {
			if (reportErrors) {
				test->report_error_at_expr("expected test expression of bool type");
			}
			return nullptr;
		}
		const Type* bodyType = body->type_syn(typeCtx);
		if (!bodyType) { return nullptr; }
		const Type* elseBodyType = elseBody->type_syn(typeCtx);
		if (!elseBodyType) { return nullptr; }
		if (!bodyType->equal(elseBodyType)) {
			if (reportErrors) {
				std::ostringstream oss;
				oss << "both branches must synthesize same type; true branch synthesizes " << bodyType
				    << " and false branch synthesizes " << elseBodyType;
				report_error_at_expr(oss.str());
			}
			return nullptr;
		}
		return bodyType;
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		if (!test->type_ana(Type::Bool(), typeCtx)) {
			test->report_error_at_expr("expected test expression of bool type");
			return false;
		}
		return body->type_ana(type, typeCtx) && elseBody->type_ana(type, typeCtx);
	}
};
