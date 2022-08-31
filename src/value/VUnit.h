#pragma once

#include "../Value.h"

class VUnit : public Value {
public:
	VUnit() {}

	void print(std::ostream& os) const override {
		os << "()";
	}

	const Type* get_type() const override {
		return Type::Unit;
	}
};
