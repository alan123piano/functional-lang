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

	void print(std::ostream& os) const {
		std::vector<std::pair<std::string, std::vector<T>>> list;
		list.reserve(ctxImpl.size());
		for (auto it = ctxImpl.begin(); it != ctxImpl.end(); ++it) {
			if (it->second.size() > 0) {
				list.push_back(*it);
			}
		}
		std::sort(list.begin(), list.end(), [](const std::pair<std::string, std::vector<T>>& a, const std::pair<std::string, std::vector<T>>& b) -> bool {
			return a.first < b.first;
		});
		os << "[";
		bool printComma = false;
		for (auto pair : list) {
			if (printComma) {
				os << ", ";
			}
			os << pair.first;
			printComma = true;
		}
		os << "]";
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

template <typename T>
std::ostream& operator<<(std::ostream& os, const Context<T>& ctx) {
	ctx.print(os);
	return os;
}
