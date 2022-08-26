#pragma once

#include "../Value.h"

class VFun : public Value {
public:
	const Expr* fun;

	VFun(const Expr* fun) : fun(fun) {}

	void print(std::ostream& os) const override {
		os << fun;
	}

	std::string type_name() const {
		// TODO: this typename should be qualified
		// (ex. int -> int)
		return "function";
	}
};
