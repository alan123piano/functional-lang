#pragma once

#include "../Value.h"

class VInt : public Value {
public:
	int value;

	VInt(int value) : value(value) {}

	void print(std::ostream& os) const override {
		os << value;
	}

	std::string type_name() const {
		return "int";
	}

	Value* negate() const override {
		return new VInt(-value);
	}

	std::optional<bool> equals(Value* other) const override {
		VInt* o = other->as<VInt>();
		if (!o) {
			return std::nullopt;
		}
		return value == o->value;
	}

	std::optional<bool> less_than(Value* other) const override {
		VInt* o = other->as<VInt>();
		if (!o) {
			return std::nullopt;
		}
		return value < o->value;
	}

	Value* plus(Value* other) const override {
		VInt* o = other->as<VInt>();
		if (!o) {
			return nullptr;
		}
		return new VInt(value + o->value);
	}

	Value* minus(Value* other) const override {
		VInt* o = other->as<VInt>();
		if (!o) {
			return nullptr;
		}
		return new VInt(value - o->value);
	}

	Value* mul(Value* other) const override {
		VInt* o = other->as<VInt>();
		if (!o) {
			return nullptr;
		}
		return new VInt(value * o->value);
	}

	Value* div(Value* other) const override {
		VInt* o = other->as<VInt>();
		if (!o) {
			return nullptr;
		}
		return new VInt(value / o->value);
	}

	Value* mod(Value* other) const override {
		VInt* o = other->as<VInt>();
		if (!o) {
			return nullptr;
		}
		return new VInt(value % o->value);
	}
};
