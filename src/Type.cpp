#include "Type.h"

const Type* Type::Int() {
	static const Type* type = new TBase("int");
	return type;
}

const Type* Type::Float() {
	static const Type* type = new TBase("float");
	return type;
}

const Type* Type::Bool() {
	static const Type* type = new TBase("bool");
	return type;
}

const Type* Type::Unit() {
	static const Type* type = new TBase("unit");
	return type;
}

std::ostream& operator<<(std::ostream& os, const Type* type) {
	type->print(os);
	return os;
}
