#pragma once

#include <string>
#include <unordered_map>
#include "parser.hpp"

class Value;

class Scope {
public:
	Scope() {}
	Scope(const Scope& other) : scope(other.scope) {}
	Scope& operator=(Scope other) {
		scope = other.scope;
		return *this;
	}

	Value* get(const EIdent* ident) const {
		auto it = scope.find(ident->value);
		if (it == scope.end() || it->second.empty()) {
			return nullptr;
		}
		return it->second.back();
	}

	void push(const EIdent* ident, Value* value) {
		scope[ident->value].push_back(value);
	}

	void pop(const EIdent* ident) {
		scope[ident->value].pop_back();
	}

private:
	std::unordered_map<std::string, std::vector<Value*>> scope;
};
