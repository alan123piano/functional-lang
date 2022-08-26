#pragma once

// unused for now (may be useful for implementing imperative language features)

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_map>

class Value;

class Scope {
private:
	using ScopeImpl = std::unordered_map<std::string, std::vector<Value*>>;

public:
	Scope() {}
	Scope(const Scope& other) : scope(other.scope) {}
	Scope& operator=(Scope other) {
		scope = other.scope;
		return *this;
	}

	void print(std::ostream& os) const {
		std::vector<ScopeImpl::const_iterator> list;
		list.reserve(scope.size());
		for (auto it = scope.begin(); it != scope.end(); ++it) {
			if (it->second.size() > 0) {
				list.push_back(it);
			}
		}
		std::sort(list.begin(), list.end(), [](const ScopeImpl::const_iterator& a, const ScopeImpl::const_iterator& b) -> bool {
			return a->first < b->first;
		});
		os << "[";
		bool printComma = false;
		for (auto it : list) {
			if (printComma) {
				os << ", ";
			}
			os << it->first;
			printComma = true;
		}
		os << "]";
	}

	Value* get(const std::string& ident) const {
		auto it = scope.find(ident);
		if (it == scope.end() || it->second.empty()) {
			return nullptr;
		}
		return it->second.back();
	}

	void push(const std::string& ident, Value* value) {
		scope[ident].push_back(value);
	}

	void pop(const std::string& ident) {
		scope[ident].pop_back();
	}

private:
	ScopeImpl scope;
};

std::ostream& operator<<(std::ostream& os, const Scope& scope) {
	scope.print(os);
	return os;
}
