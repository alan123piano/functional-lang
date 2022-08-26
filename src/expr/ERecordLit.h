#pragma once

#include "../Expr.h"

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

	Value* eval() const override {
		// TODO: implement
		// add VRecord?
		return nullptr;
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
