#pragma once

#include "../Value.h"

class VBool : public Value {
public:
	bool value;

	VBool(bool value) : value(value) {}

	void print(std::ostream& os) const override {
		os << (value ? "true" : "false");
	}

	std::string type_name() const {
		return "bool";
	}

	std::optional<bool> equals(Value* other) const override {
		VBool* otherBool = other->as<VBool>();
		if (!otherBool) {
			return std::nullopt;
		}
		return value == otherBool->value;
	}
};
