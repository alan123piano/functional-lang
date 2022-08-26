#pragma once

#include "../Value.h"

class VUnit : public Value {
public:
	VUnit() {}

	void print(std::ostream& os) const override {
		os << "()";
	}

	std::string type_name() const {
		return "unit";
	}

	std::optional<bool> equals(Value* other) const override {
		VUnit* otherUnit = other->as<VUnit>();
		if (!otherUnit) {
			return std::nullopt;
		}
		return true;
	}
};
