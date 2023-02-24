#include "Expr.h"

std::ostream& operator<<(std::ostream& os, const Expr* expr) {
	expr->print(os);
	return os;
}
