#pragma once

#include <sstream>
#include <optional>

class EFun;

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

class VFun : public Value {
public:
	EFun* fun;

	VFun(EFun* fun) : fun(fun) {}

	void print(std::ostream& os) const override {
		os << fun;
	}

	std::string type_name() const {
		// TODO: this typename should be qualified
		// (ex. int -> int)
		return "function";
	}
};

std::ostream& operator<<(std::ostream& os, const Value* value) {
	value->print(os);
	return os;
}
