#pragma once

#include "../Value.h"

class VBool : public Value {
public:
	bool value;

	VBool(bool value) : value(value) {}

	void print(std::ostream& os) const override {
		os << (value ? "true" : "false");
	}

	const Type* get_type() const override {
		return Type::Bool();
	}
};
