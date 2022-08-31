#pragma once

#include "../Value.h"

class VFloat : public Value {
public:
	double value;

	VFloat(double value) : value(value) {}

	void print(std::ostream& os) const override {
		os << value;
	}

	const Type* get_type() const override {
		return Type::Float;
	}
};
