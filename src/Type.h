#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_map>

class Type {
public:
	Type() {}
	virtual ~Type() {}

	virtual bool equal(const Type* other) const = 0;
	virtual void print(std::ostream& os) const = 0;

	static const Type* Int();
	static const Type* Float();
	static const Type* Bool();
	static const Type* Unit();

	template <typename T>
	const T* as() const {
		return dynamic_cast<const T*>(this);
	}
};

class TBase : public Type {
public:
	std::string name;

	TBase(std::string name) : name(std::move(name)) {}

	bool equal(const Type* other) const override {
		if (!other->as<TBase>()) { return false; }
		return name == other->as<TBase>()->name;
	}

	void print(std::ostream& os) const override {
		os << name;
	}
};

class TArrow : public Type {
public:
	const Type* left;
	const Type* right;

	TArrow(const Type* left, const Type* right)
		: left(left), right(right) {}

	bool equal(const Type* other) const override {
		const TArrow* o = other->as<TArrow>();
		if (!o) { return false; }
		return left->equal(o->left) && right->equal(o->right);
	}

	void print(std::ostream& os) const override {
		left->print(os);
		os << " -> ";
		right->print(os);
	}
};

class TTuple : public Type {
public:
	std::vector<const Type*> types;

	TTuple(std::vector<const Type*> types)
		: types(std::move(types)) {}

	bool equal(const Type* other) const override {
		if (!other->as<TTuple>()) { return false; }
		const TTuple* o = other->as<TTuple>();
		if (types.size() != o->types.size()) { return false; }
		for (size_t i = 0; i < types.size(); ++i) {
			if (!types[i]->equal(o->types[i])) { return false; }
		}
		return true;
	}

	void print(std::ostream& os) const override {
		if (types.empty()) {
			throw std::runtime_error("Attempting to print empty tuple type");
		}
		os << "(";
		types[0]->print(os);
		for (size_t i = 1; i < types.size(); ++i) {
			os << " * ";
			types[i]->print(os);
		}
		os << ")";
	}
};

class TVariant : public Type {
public:
	struct Case {
		std::string tag;
		const Type* type;
	};

	std::string name;
	std::vector<Case> cases;

	TVariant(std::string name, std::vector<Case> cases)
		: name(std::move(name)), cases(std::move(cases)) {}

	bool equal(const Type* other) const override {
		if (!other->as<TVariant>()) { return false; }
		return name == other->as<TVariant>()->name;
	}

	void print(std::ostream& os) const override {
		os << name;
	}
};

class TRecord : public Type {
public:
	struct Field {
		std::string ident;
		const Type* type;
	};

	std::string name;
	std::unordered_map<std::string, const Type*> fields;
	std::vector<std::string> idents; // original order of idents

	TRecord(std::string name, const std::vector<Field>& orderedFields)
		: name(std::move(name)) {
		for (const Field& field : orderedFields) {
			if (fields.find(field.ident) != fields.end()) {
				throw std::runtime_error("Duplicate field in record type declaration");
			}
			fields.insert({ field.ident, field.type });
			idents.push_back(field.ident);
		}
	}

	bool equal(const Type* other) const override {
		if (!other->as<TRecord>()) { return false; }
		return name == other->as<TRecord>()->name;
	}

	void print(std::ostream& os) const override {
		os << name;
	}
};

std::ostream& operator<<(std::ostream& os, const Type* type);
