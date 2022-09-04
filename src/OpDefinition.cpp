#include "OpDefinition.h"

const std::unordered_map<UnaryOpSignature, UnaryOpResult> OpDefinition::unaryOpDefs{
	{
		{ TokenType::Minus, Type::Int() },
		{
			Type::Int(), [](const Value* v) -> Value* {
				return new VInt(-v->as<VInt>()->value);
			}
		}
	},
	{
		{ TokenType::Minus, Type::Float() },
		{
			Type::Float(), [](const Value* v) -> Value* {
				return new VFloat(-v->as<VFloat>()->value);
			}
		}
	},
	{
		{ TokenType::Not, Type::Bool() },
		{
			Type::Bool(), [](const Value* v) -> Value* {
				return new VBool(!v->as<VBool>()->value);
			}
		}
	},
};

// core operators: '=', '<', '+', '-', '*', '/'
const std::unordered_map<BinaryOpSignature, BinaryOpResult> OpDefinition::binaryOpDefs{
	{
		{ Type::Int(), TokenType::Equals, Type::Int() },
		{
			Type::Bool(), [](const Value* l, const Value* r) -> Value* {
				return new VBool(l->as<VInt>()->value == r->as<VInt>()->value);
			}
		}
	},
	{
		{ Type::Float(), TokenType::Equals, Type::Float() },
		{
			Type::Bool(), [](const Value* l, const Value* r) -> Value* {
				return new VBool(l->as<VFloat>()->value == r->as<VFloat>()->value);
			}
		}
	},
	{
		{ Type::Bool(), TokenType::Equals, Type::Bool() },
		{
			Type::Bool(), [](const Value* l, const Value* r) -> Value* {
				return new VBool(l->as<VBool>()->value == r->as<VBool>()->value);
			}
		}
	},
	{
		{ Type::Unit(), TokenType::Equals, Type::Unit() },
		{
			Type::Bool(), [](const Value* l, const Value* r) -> Value* {
				return new VBool(true);
			}
		}
	},
	{
		{ Type::Int(), TokenType::Lt, Type::Int() },
		{
			Type::Bool(), [](const Value* l, const Value* r) -> Value* {
				return new VBool(l->as<VInt>()->value < r->as<VInt>()->value);
			}
		}
	},
	{
		{ Type::Float(), TokenType::Lt, Type::Float() },
		{
			Type::Bool(), [](const Value* l, const Value* r) -> Value* {
				return new VBool(l->as<VFloat>()->value < r->as<VFloat>()->value);
			}
		}
	},
	{
		{ Type::Bool(), TokenType::And, Type::Bool() },
		{
			Type::Bool(), [](const Value* l, const Value* r) -> Value* {
				return new VBool(l->as<VBool>()->value&& r->as<VBool>()->value);
			}
		}
	},
	{
		{ Type::Bool(), TokenType::Or, Type::Bool() },
		{
			Type::Bool(), [](const Value* l, const Value* r) -> Value* {
				return new VBool(l->as<VBool>()->value || r->as<VBool>()->value);
			}
		}
	},
	{
		{ Type::Int(), TokenType::Plus, Type::Int() },
		{
			Type::Int(), [](const Value* l, const Value* r) -> Value* {
				return new VInt(l->as<VInt>()->value + r->as<VInt>()->value);
			}
		}
	},
	{
		{ Type::Float(), TokenType::Plus, Type::Float() },
		{
			Type::Float(), [](const Value* l, const Value* r) -> Value* {
				return new VFloat(l->as<VFloat>()->value + r->as<VFloat>()->value);
			}
		}
	},
	{
		{ Type::Int(), TokenType::Minus, Type::Int() },
		{
			Type::Int(), [](const Value* l, const Value* r) -> Value* {
				return new VInt(l->as<VInt>()->value - r->as<VInt>()->value);
			}
		}
	},
	{
		{ Type::Float(), TokenType::Minus, Type::Float() },
		{
			Type::Float(), [](const Value* l, const Value* r) -> Value* {
				return new VFloat(l->as<VFloat>()->value - r->as<VFloat>()->value);
			}
		}
	},
	{
		{ Type::Int(), TokenType::Mul, Type::Int() },
		{
			Type::Int(), [](const Value* l, const Value* r) -> Value* {
				return new VInt(l->as<VInt>()->value * r->as<VInt>()->value);
			}
		}
	},
	{
		{ Type::Float(), TokenType::Mul, Type::Float() },
		{
			Type::Float(), [](const Value* l, const Value* r) -> Value* {
				return new VFloat(l->as<VFloat>()->value * r->as<VFloat>()->value);
			}
		}
	},
	{
		{ Type::Int(), TokenType::Div, Type::Int() },
		{
			Type::Int(), [](const Value* l, const Value* r) -> Value* {
				return new VInt(l->as<VInt>()->value / r->as<VInt>()->value);
			}
		}
	},
	{
		{ Type::Float(), TokenType::Div, Type::Float() },
		{
			Type::Float(), [](const Value* l, const Value* r) -> Value* {
				return new VFloat(l->as<VFloat>()->value / r->as<VFloat>()->value);
			}
		}
	},
	{
		{ Type::Int(), TokenType::Mod, Type::Int() },
		{
			Type::Int(), [](const Value* l, const Value* r) -> Value* {
				return new VInt(l->as<VInt>()->value % r->as<VInt>()->value);
			}
		}
	}
};
