#pragma once

#include "../Value.h"

class VInt : public Value {
public:
	long long value;

	VInt(long long value) : value(value) {}

	void print(std::ostream& os) const override {
		os << value;
	}

	const Type* get_type() const override {
		return Type::Int;
	}
};
