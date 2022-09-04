#pragma once

#include <sstream>
#include <optional>
#include "Type.h"

class Value {
public:
	Value() {}
	virtual ~Value() {}
	virtual void print(std::ostream& os) const = 0;
	virtual const Type* get_type() const = 0;

	template <typename T>
	const T* as() const {
		return dynamic_cast<const T*>(this);
	}
};

std::ostream& operator<<(std::ostream& os, const Value* value);
