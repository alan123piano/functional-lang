#pragma once

#include <sstream>
#include "token.hpp"
#include "location.hpp"

class EIdent;

class Expr {
public:
	Location loc;

	// AST locations always have length 0 (AST nodes can be multi-line)
	Expr(const Location& loc) : loc({ loc.line, loc.colStart, loc.colStart }) {}
	virtual ~Expr() {}
	virtual void print(std::ostream& os) const = 0;
	virtual Expr* copy() const = 0;
	virtual Expr* subst(const std::string& subIdent, const Expr* subExpr) const = 0;

	template <typename T>
	T* as() {
		return dynamic_cast<T*>(this);
	}

protected:
	static void try_print(std::ostream& os, Expr* expr) {
		if (expr) {
			expr->print(os);
		} else {
			os << "<error>";
		}
	}
};

class EIntLit : public Expr {
public:
	long long value;

	EIntLit(const Location& loc, long long value)
		: Expr(loc), value(value) {}

	void print(std::ostream& os) const override {
		os << value;
	}

	Expr* copy() const override {
		return new EIntLit(loc, value);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		return copy();
	}
};

class EBoolLit : public Expr {
public:
	bool value;

	EBoolLit(const Location& loc, bool value)
		: Expr(loc), value(value) {}

	void print(std::ostream& os) const override {
		os << (value ? "true" : "false");
	}

	Expr* copy() const override {
		return new EBoolLit(loc, value);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		return copy();
	}
};

class EIdent : public Expr {
public:
	std::string value;

	EIdent(const Location& loc, std::string value)
		: Expr(loc), value(std::move(value)) {}

	void print(std::ostream& os) const override {
		os << value;
	}

	Expr* copy() const override {
		return new EIdent(loc, value);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		if (value == subIdent) {
			return subExpr->copy();
		} else {
			return copy();
		}
	}
};

class ELet : public Expr {
public:
	std::string ident;
	Expr* value;
	Expr* body;

	ELet(const Location& loc, std::string ident, Expr* value, Expr* body)
		: Expr(loc), ident(std::move(ident)), value(value), body(body) {}

	void print(std::ostream& os) const override {
		os << "let " << ident << " = (";
		try_print(os, value);
		os << ") in (";
		try_print(os, body);
		os << ")";
	}

	Expr* copy() const override {
		return new ELet(loc, ident, value->copy(), body->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newValue = value->subst(subIdent, subExpr);
		Expr* newBody;
		if (subIdent != ident) {
			newBody = body->subst(subIdent, subExpr);
		} else {
			newBody = body->copy();
		}
		return new ELet(loc, ident, newValue, newBody);
	}
};

class EIf : public Expr {
public:
	Expr* test;
	Expr* body;
	Expr* elseBody;

	EIf(const Location& loc, Expr* test, Expr* body, Expr* elseBody)
		: Expr(loc), test(test), body(body), elseBody(elseBody) {}

	void print(std::ostream& os) const override {
		os << "if (";
		try_print(os, test);
		os << ") then (";
		try_print(os, body);
		os << ") else (";
		try_print(os, elseBody);
		os << ")";
	}

	Expr* copy() const override {
		return new EIf(loc, test->copy(), body->copy(), elseBody->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newTest = test->subst(subIdent, subExpr);
		Expr* newBody = body->subst(subIdent, subExpr);
		Expr* newElseBody = elseBody->subst(subIdent, subExpr);
		return new EIf(loc, newTest, newBody, newElseBody);
	}
};

class EFun : public Expr {
public:
	std::string ident;
	Expr* body;

	EFun(const Location& loc, std::string ident, Expr* body)
		: Expr(loc), ident(std::move(ident)), body(body) {}

	void print(std::ostream& os) const override {
		os << "fun " << ident << " -> ";
		try_print(os, body);
	}

	Expr* copy() const override {
		return new EFun(loc, ident, body->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newBody;
		if (subIdent != ident) {
			newBody = body->subst(subIdent, subExpr);
		} else {
			newBody = body->copy();
		}
		return new EFun(loc, ident, newBody);
	}
};

class EFix : public Expr {
public:
	std::string ident;
	Expr* body;

	EFix(const Location& loc, std::string ident, Expr* body)
		: Expr(loc), ident(std::move(ident)), body(body) {}

	void print(std::ostream& os) const override {
		os << "fix " << ident << " -> (";
		try_print(os, body);
		os << ")";
	}

	Expr* copy() const override {
		return new EFix(loc, ident, body->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newBody;
		if (subIdent != ident) {
			newBody = body->subst(subIdent, subExpr);
		} else {
			newBody = body->copy();
		}
		return new EFix(loc, ident, newBody);
	}
};

class EFunAp : public Expr {
public:
	Expr* fun;
	Expr* arg;

	EFunAp(const Location& loc, Expr* fun, Expr* arg)
		: Expr(loc), fun(fun), arg(arg) {}

	void print(std::ostream& os) const override {
		os << "(";
		try_print(os, fun);
		os << ") (";
		try_print(os, arg);
		os << ")";
	}

	Expr* copy() const override {
		return new EFunAp(loc, fun->copy(), arg->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newFun = fun->subst(subIdent, subExpr);
		Expr* newArg = arg->subst(subIdent, subExpr);
		return new EFunAp(loc, newFun, newArg);
	}
};

class EUnaryOp : public Expr {
public:
	Token op;
	Expr* right;

	EUnaryOp(const Location& loc, Token op, Expr* right)
		: Expr(loc), op(op), right(right) {}

	void print(std::ostream& os) const override {
		os << op << "(";
		try_print(os, right);
		os << ")";
	}

	Expr* copy() const override {
		return new EUnaryOp(loc, op, right->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newRight = right->subst(subIdent, subExpr);
		return new EUnaryOp(loc, op, newRight);
	}
};

class EBinOp : public Expr {
public:
	Expr* left;
	Token op;
	Expr* right;

	EBinOp(const Location& loc, Expr* left, Token op, Expr* right)
		: Expr(loc), left(left), op(op), right(right) {}

	void print(std::ostream& os) const override {
		os << "(";
		try_print(os, left);
		os << ") " << op << " (";
		try_print(os, right);
		os << ")";
	}

	Expr* copy() const override {
		return new EBinOp(loc, left->copy(), op, right->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newLeft = left->subst(subIdent, subExpr);
		Expr* newRight = right->subst(subIdent, subExpr);
		return new EBinOp(loc, newLeft, op, newRight);
	}
};

std::ostream& operator<<(std::ostream& os, const Expr* expr) {
	expr->print(os);
	return os;
}
