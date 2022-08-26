#pragma once

#include <sstream>
#include <optional>

class Value {
public:
	Value() {}
	virtual ~Value() {}
	virtual void print(std::ostream& os) const = 0;
	virtual std::string type_name() const = 0;

	virtual Value* negate() const {
		return nullptr;
	}

	virtual std::optional<bool> equals(Value* other) const {
		return std::nullopt;
	}

	virtual std::optional<bool> less_than(Value* other) const {
		return std::nullopt;
	}

	virtual Value* plus(Value* other) const {
		return nullptr;
	}

	virtual Value* minus(Value* other) const {
		return nullptr;
	}

	virtual Value* mul(Value* other) const {
		return nullptr;
	}

	virtual Value* div(Value* other) const {
		return nullptr;
	}

	virtual Value* mod(Value* other) const {
		return nullptr;
	}

	template <typename T>
	T* as() {
		return dynamic_cast<T*>(this);
	}
};

std::ostream& operator<<(std::ostream& os, const Value* value) {
	value->print(os);
	return os;
}
