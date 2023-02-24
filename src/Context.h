#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <unordered_map>

// use for typing context (Context<const Type*>) or imperative scope (Context<const Value*>)
template <typename T>
class Context {
private:
	using ContextImpl = std::unordered_map<std::string, std::vector<T>>;

public:
	Context() {}
	Context(const Context& other) : ctxImpl(other.ctxImpl) {}
	Context& operator=(const Context& other) {
		ctxImpl = other.ctxImpl;
		return *this;
	}

	T get(const std::string& ident) const {
		auto it = ctxImpl.find(ident);
		if (it == ctxImpl.end() || it->second.empty()) {
			return nullptr;
		}
		return it->second.back();
	}

	void push(const std::string& ident, T value) {
		ctxImpl[ident].push_back(value);
	}

	void pop(const std::string& ident) {
		ctxImpl[ident].pop_back();
	}

private:
	ContextImpl ctxImpl;
};
