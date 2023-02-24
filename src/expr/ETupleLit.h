#pragma once

#include "../Expr.h"

class ETupleLit : public Expr {
public:
	std::vector<Expr*> fields;

	ETupleLit(const Location& loc, std::vector<Expr*> fields)
		: Expr(loc), fields(std::move(fields)) {}

	void print(std::ostream& os) const override {
		os << "(";
		bool printComma = false;
		for (const Expr* expr : fields) {
			if (printComma) {
				os << ", ";
			}
			printComma = true;
			os << expr;
		}
		os << ")";
	}

	Expr* copy() const override {
		// TODO
		return nullptr;
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		// TODO
		return nullptr;
	}

	Value* eval() const override {
		// TODO
		return nullptr;
	}

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		// TODO
		return nullptr;
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		// TODO
		return false;
	}
};
