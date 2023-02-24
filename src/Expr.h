#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include "Type.h"
#include "Token.h"
#include "Value.h"
#include "Source.h"
#include "Context.h"

class Expr {
public:
	Location loc;

	// AST locations always have length 0 (AST nodes can be multi-line)
	Expr(const Location& loc) : loc({ loc.source, loc.line, loc.colStart, loc.colStart }) {}
	virtual ~Expr() {}
	virtual void print(std::ostream& os) const = 0;
	virtual Expr* copy() const = 0;
	virtual Expr* subst(const std::string& subIdent, const Expr* subExpr) const = 0;
	virtual Value* eval() const = 0;
	// bidirectional type synthesis & analysis
	virtual const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const = 0;
	virtual bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const = 0;
	// virtual const Type* type_infer()

	void report_error_at_expr(std::string error) const {
		loc.source->report_error(loc.line, loc.colStart, 0, std::move(error));
	}

	template <typename T>
	const T* as() const {
		return dynamic_cast<const T*>(this);
	}
};

std::ostream& operator<<(std::ostream& os, const Expr* expr);
