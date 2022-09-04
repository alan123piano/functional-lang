#pragma once

#include <sstream>
#include <utility>
#include <unordered_map>
#include "Type.h"
#include "Token.h"
#include "Value.h"
#include "value/VBool.h"
#include "value/VFloat.h"
#include "value/VInt.h"

// define unary and binary operations

struct UnaryOpSignature {
	TokenType op;
	const Type* right;
	bool operator==(const UnaryOpSignature& o) const {
		return op == o.op && right->equal(o.right);
	}
};

namespace std {
template<> struct hash<UnaryOpSignature> {
	std::size_t operator()(const UnaryOpSignature& sig) const noexcept {
		std::ostringstream os;
		os << sig.op << sig.right;
		return std::hash<std::string>()(os.str());
	}
};
}

struct BinaryOpSignature {
	const Type* left;
	TokenType op;
	const Type* right;
	bool operator==(const BinaryOpSignature& o) const {
		return left->equal(o.left) && op == o.op && right->equal(o.right);
	}
};

namespace std {
template<> struct hash<BinaryOpSignature> {
	std::size_t operator()(const BinaryOpSignature& sig) const noexcept {
		std::ostringstream os;
		os << sig.left << sig.op << sig.right;
		return std::hash<std::string>()(os.str());
	}
};
}

struct UnaryOpResult {
	const Type* type;
	std::function<Value*(const Value*)> fun;
};

struct BinaryOpResult {
	const Type* type;
	std::function<Value*(const Value*, const Value*)> fun;
};

class OpDefinition {
public:
	static const Type* unary_op_type(TokenType op, const Type* rtype) {
		if (!rtype) { return nullptr; }
		auto it = unaryOpDefs.find({ op, rtype });
		if (it == unaryOpDefs.end()) { return nullptr; }
		return it->second.type;
	}

	static const Type* binary_op_type(const Type* ltype, TokenType op, const Type* rtype) {
		if (!ltype || !rtype) { return nullptr; }
		switch (op) {
		// hard-coding derivative operators (see binary_op_result for eval)
		case TokenType::NotEquals: {
			auto eqIt = binaryOpDefs.find({ ltype, TokenType::Equals, rtype });
			if (eqIt == binaryOpDefs.end()) { return nullptr; }
			return Type::Bool();
		}
		case TokenType::Gt: {
			auto ltIt = binaryOpDefs.find({ ltype, TokenType::Lt, rtype });
			if (ltIt == binaryOpDefs.end()) { return nullptr; }
			return Type::Bool();
		}
		case TokenType::Leq: {
			auto ltIt = binaryOpDefs.find({ ltype, TokenType::Lt, rtype });
			if (ltIt == binaryOpDefs.end()) { return nullptr; }
			auto eqIt = binaryOpDefs.find({ ltype, TokenType::Equals, rtype });
			if (eqIt == binaryOpDefs.end()) { return nullptr; }
			return Type::Bool();
		}
		case TokenType::Geq: {
			auto ltIt = binaryOpDefs.find({ ltype, TokenType::Lt, rtype });
			if (ltIt == binaryOpDefs.end()) { return nullptr; }
			auto eqIt = binaryOpDefs.find({ ltype, TokenType::Equals, rtype });
			if (eqIt == binaryOpDefs.end()) { return nullptr; }
			return Type::Bool();
		}
		default: {
			auto it = binaryOpDefs.find({ ltype, op, rtype });
			if (it == binaryOpDefs.end()) { return nullptr; }
			return it->second.type;
		}
		}
	}

	static Value* unary_op_result(TokenType op, const Value* right) {
		const Type* rtype = right->get_type();
		if (!rtype) {
			throw std::runtime_error("UnaryOpResult: Failed to get right type");
		}
		auto it = unaryOpDefs.find({ op, rtype });
		if (it == unaryOpDefs.end()) { return nullptr; }
		return it->second.fun(right);
	}

	static Value* binary_op_result(const Value* left, TokenType op, const Value* right) {
		const Type* ltype = left->get_type();
		if (!ltype) {
			throw std::runtime_error("BinaryOpResult: Failed to get left type");
		}
		const Type* rtype = right->get_type();
		if (!rtype) {
			throw std::runtime_error("BinaryOpResult: Failed to get right type");
		}
		switch (op) {
		case TokenType::NotEquals: {
			auto eqIt = binaryOpDefs.find({ ltype, TokenType::Equals, rtype });
			if (eqIt == binaryOpDefs.end()) { return nullptr; }
			return new VBool(!eqIt->second.fun(left, right)->as<VBool>()->value);
		}
		case TokenType::Gt: {
			auto ltIt = binaryOpDefs.find({ ltype, TokenType::Lt, rtype });
			if (ltIt == binaryOpDefs.end()) { return nullptr; }
			return ltIt->second.fun(right, left);
		}
		case TokenType::Leq: {
			auto ltIt = binaryOpDefs.find({ ltype, TokenType::Lt, rtype });
			if (ltIt == binaryOpDefs.end()) { return nullptr; }
			auto eqIt = binaryOpDefs.find({ ltype, TokenType::Equals, rtype });
			if (eqIt == binaryOpDefs.end()) { return nullptr; }
			return new VBool(ltIt->second.fun(left, right)->as<VBool>()->value ||
			                 eqIt->second.fun(left, right)->as<VBool>()->value);
		}
		case TokenType::Geq: {
			auto ltIt = binaryOpDefs.find({ ltype, TokenType::Lt, rtype });
			if (ltIt == binaryOpDefs.end()) { return nullptr; }
			auto eqIt = binaryOpDefs.find({ ltype, TokenType::Equals, rtype });
			if (eqIt == binaryOpDefs.end()) { return nullptr; }
			return new VBool(ltIt->second.fun(right, left)->as<VBool>()->value ||
			                 eqIt->second.fun(left, right)->as<VBool>()->value);
		}
		default: {
			auto it = binaryOpDefs.find({ ltype, op, rtype });
			if (it == binaryOpDefs.end()) { return nullptr; }
			return it->second.fun(left, right);
		}
		}
	}

private:
	static const std::unordered_map<UnaryOpSignature, UnaryOpResult> unaryOpDefs;
	static const std::unordered_map<BinaryOpSignature, BinaryOpResult> binaryOpDefs;
};
