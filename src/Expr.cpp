#include "Expr.h"

std::ostream& operator<<(std::ostream& os, const Expr* expr) {
	Expr::print(os, expr);
	return os;
}
