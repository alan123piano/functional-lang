#pragma once

#include <vector>
#include <sstream>
#include <unordered_map>
#include "type.hpp"
#include "token.hpp"
#include "source.hpp"

class Expr {
public:
	Location loc;
	const Type* typeAnn = nullptr;

	// AST locations always have length 0 (AST nodes can be multi-line)
	Expr(const Location& loc, const Type* typeAnn)
		: loc({ loc.source, loc.line, loc.colStart, loc.colStart }), typeAnn(typeAnn) {}
	virtual ~Expr() {}
	virtual Expr* copy() const = 0;
	virtual Expr* subst(const std::string& subIdent, const Expr* subExpr) const = 0;

	void report_error_at_expr(std::string error) {
		loc.source->report_error(loc.line, loc.colStart, 0, std::move(error));
	}

	static void print(std::ostream& os, const Expr* expr) {
		if (expr) {
			expr->print_impl(os);
			if (expr->typeAnn) {
				os << " : ";
				expr->typeAnn->print(os);
			}
		} else {
			os << "<error>";
		}
	}

	template <typename T>
	T* as() {
		return dynamic_cast<T*>(this);
	}

private:
	virtual void print_impl(std::ostream& os) const = 0;
};

class EIntLit : public Expr {
public:
	int value;

	EIntLit(const Location& loc, const Type* typeAnn, int value)
		: Expr(loc, typeAnn), value(value) {}

	Expr* copy() const override {
		return new EIntLit(loc, typeAnn, value);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		return copy();
	}

	void print_impl(std::ostream& os) const override {
		os << value;
	}
};

class EBoolLit : public Expr {
public:
	bool value;

	EBoolLit(const Location& loc, const Type* typeAnn, bool value)
		: Expr(loc, typeAnn), value(value) {}

	Expr* copy() const override {
		return new EBoolLit(loc, typeAnn, value);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		return copy();
	}

	void print_impl(std::ostream& os) const override {
		os << (value ? "true" : "false");
	}
};

class EUnitLit : public Expr {
public:
	EUnitLit(const Location& loc, const Type* typeAnn)
		: Expr(loc, typeAnn) {}

	Expr* copy() const override {
		return new EUnitLit(loc, typeAnn);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		return copy();
	}

	void print_impl(std::ostream& os) const override {
		os << "()";
	}
};

class ERecordLit : public Expr {
public:
	struct Field {
		std::string ident;
		Expr* expr;
	};

	std::unordered_map<std::string, Expr*> fields;
	std::vector<std::string> idents; // original order of idents

	// for Record literals, a type is always expected to be provided
	// if the Record literal expr has a type annotation, that will be used
	// instead, the type must be inferred from the fields
	// this constructor verifies that the field identifiers are consistent,
	// but does NOT validate types of field expressions
	ERecordLit(const Location& loc, const Type* typeAnn, const std::vector<Field>& orderedFields)
		: Expr(loc, typeAnn) {
		for (const Field& field : orderedFields) {
			if (fields.find(field.ident) != fields.end()) {
				throw std::runtime_error("Duplicate field in record literal expression");
			}
			fields.insert({ field.ident, field.expr });
			idents.push_back(field.ident);
		}
	}

	Expr* copy() const override {
		std::vector<Field> fieldsCopy;
		for (const std::string& ident : idents) {
			fieldsCopy.push_back({ ident, fields.at(ident)->copy() });
		}
		return new ERecordLit(loc, typeAnn, fieldsCopy);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		std::vector<Field> fieldsCopy;
		for (const std::string& ident : idents) {
			fieldsCopy.push_back({ ident, fields.at(ident)->subst(subIdent, subExpr) });
		}
		return new ERecordLit(loc, typeAnn, fieldsCopy);
	}

	void print_impl(std::ostream& os) const override {
		os << "{ ";
		bool printComma = false;
		for (const std::string& ident : idents) {
			if (printComma) {
				os << ", ";
			}
			printComma = true; // print comma after first
			os << ident << " = ";
			print(os, fields.at(ident));
		}
		os << " }";
	}
};

class EVar : public Expr {
public:
	std::string value;

	EVar(const Location& loc, const Type* typeAnn, std::string value)
		: Expr(loc, typeAnn), value(std::move(value)) {}

	Expr* copy() const override {
		return new EVar(loc, typeAnn, value);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		if (value == subIdent) {
			return subExpr->copy();
		} else {
			return copy();
		}
	}

	void print_impl(std::ostream& os) const override {
		os << value;
	}
};

class ELet : public Expr {
public:
	EVar* ident;
	Expr* value;
	Expr* body;

	ELet(const Location& loc, const Type* typeAnn, EVar* ident, Expr* value, Expr* body)
		: Expr(loc, typeAnn), ident(ident), value(value), body(body) {}

	Expr* copy() const override {
		return new ELet(loc, typeAnn, ident, value->copy(), body->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newValue = value->subst(subIdent, subExpr);
		Expr* newBody;
		if (subIdent != ident->value) {
			newBody = body->subst(subIdent, subExpr);
		} else {
			newBody = body->copy();
		}
		return new ELet(loc, typeAnn, ident, newValue, newBody);
	}

	void print_impl(std::ostream& os) const override {
		os << "(let ";
		print(os, ident);
		os << " = ";
		print(os, value);
		os << " in ";
		print(os, body);
		os << ")";
	}
};

class EIf : public Expr {
public:
	Expr* test;
	Expr* body;
	Expr* elseBody;

	EIf(const Location& loc, const Type* typeAnn, Expr* test, Expr* body, Expr* elseBody)
		: Expr(loc, typeAnn), test(test), body(body), elseBody(elseBody) {}

	Expr* copy() const override {
		return new EIf(loc, typeAnn, test->copy(), body->copy(), elseBody->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newTest = test->subst(subIdent, subExpr);
		Expr* newBody = body->subst(subIdent, subExpr);
		Expr* newElseBody = elseBody->subst(subIdent, subExpr);
		return new EIf(loc, typeAnn, newTest, newBody, newElseBody);
	}

	void print_impl(std::ostream& os) const override {
		os << "(if ";
		print(os, test);
		os << " then ";
		print(os, body);
		os << " else ";
		print(os, elseBody);
		os << ")";
	}
};

class EFun : public Expr {
public:
	EVar* ident;
	Expr* body;

	EFun(const Location& loc, const Type* typeAnn, EVar* ident, Expr* body)
		: Expr(loc, typeAnn), ident(ident), body(body) {}

	Expr* copy() const override {
		return new EFun(loc, typeAnn, ident, body->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newBody;
		if (subIdent != ident->value) {
			newBody = body->subst(subIdent, subExpr);
		} else {
			newBody = body->copy();
		}
		return new EFun(loc, typeAnn, ident, newBody);
	}

	void print_impl(std::ostream& os) const override {
		os << "(fun ";
		print(os, ident);
		os << " -> ";
		print(os, body);
		os << ")";
	}
};

class EFix : public Expr {
public:
	EVar* ident;
	Expr* body;

	EFix(const Location& loc, const Type* typeAnn, EVar* ident, Expr* body)
		: Expr(loc, typeAnn), ident(ident), body(body) {}

	Expr* copy() const override {
		return new EFix(loc, typeAnn, ident, body->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newBody;
		if (subIdent != ident->value) {
			newBody = body->subst(subIdent, subExpr);
		} else {
			newBody = body->copy();
		}
		return new EFix(loc, typeAnn, ident, newBody);
	}

	void print_impl(std::ostream& os) const override {
		os << "(fix ";
		print(os, ident);
		os << " -> ";
		print(os, body);
		os << ")";
	}
};

class EFunAp : public Expr {
public:
	Expr* fun;
	Expr* arg;

	EFunAp(const Location& loc, const Type* typeAnn, Expr* fun, Expr* arg)
		: Expr(loc, typeAnn), fun(fun), arg(arg) {}

	Expr* copy() const override {
		return new EFunAp(loc, typeAnn, fun->copy(), arg->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newFun = fun->subst(subIdent, subExpr);
		Expr* newArg = arg->subst(subIdent, subExpr);
		return new EFunAp(loc, typeAnn, newFun, newArg);
	}

	void print_impl(std::ostream& os) const override {
		os << "(";
		print(os, fun);
		os << " ";
		print(os, arg);
		os << ")";
	}
};

class EUnaryOp : public Expr {
public:
	Token op;
	Expr* right;

	EUnaryOp(const Location& loc, const Type* typeAnn, Token op, Expr* right)
		: Expr(loc, typeAnn), op(op), right(right) {}

	Expr* copy() const override {
		return new EUnaryOp(loc, typeAnn, op, right->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newRight = right->subst(subIdent, subExpr);
		return new EUnaryOp(loc, typeAnn, op, newRight);
	}

	void print_impl(std::ostream& os) const override {
		os << op;
		print(os, right);
	}
};

class EBinOp : public Expr {
public:
	Expr* left;
	Token op;
	Expr* right;

	EBinOp(const Location& loc, const Type* typeAnn, Expr* left, Token op, Expr* right)
		: Expr(loc, typeAnn), left(left), op(op), right(right) {}

	Expr* copy() const override {
		return new EBinOp(loc, typeAnn, left->copy(), op, right->copy());
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		Expr* newLeft = left->subst(subIdent, subExpr);
		Expr* newRight = right->subst(subIdent, subExpr);
		return new EBinOp(loc, typeAnn, newLeft, op, newRight);
	}

	void print_impl(std::ostream& os) const override {
		os << "(";
		print(os, left);
		os << " " << op << " ";
		print(os, right);
		os << ")";
	}
};

std::ostream& operator<<(std::ostream& os, const Expr* expr) {
	Expr::print(os, expr);
	return os;
}
