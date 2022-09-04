#include "Value.h"

std::ostream& operator<<(std::ostream& os, const Value* value) {
	value->print(os);
	return os;
}
