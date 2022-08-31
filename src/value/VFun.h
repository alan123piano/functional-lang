#pragma once

#include "../Value.h"

class VFun : public Value {
public:
	const Expr* fun;

	VFun(const Expr* fun) : fun(fun) {}

	void print(std::ostream& os) const override {
		os << fun;
	}

	const Type* get_type() const override {
		return fun->type_syn(Context<const Type*>());
	}
};
