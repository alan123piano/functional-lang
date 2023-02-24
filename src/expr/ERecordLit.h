#pragma once

#include "../Expr.h"

class ERecordLit : public Expr {
public:
	struct Field {
		std::string ident;
		Expr* expr;
	};

	std::unordered_map<std::string, Expr*> fields;
	const Type* type;

	// a type must always be provided (typically inferred from identifier set)
	ERecordLit(const Location& loc, const std::vector<Field>& orderedFields, const Type* type)
		: Expr(loc), type(type) {
		for (const Field& field : orderedFields) {
			if (fields.find(field.ident) != fields.end()) {
				throw std::runtime_error("Duplicate field in record literal expression");
			}
			fields.insert({ field.ident, field.expr });
			idents.push_back(field.ident);
		}
	}

	void print(std::ostream& os) const override {
		os << "{ ";
		bool printComma = false;
		for (const std::string& ident : idents) {
			if (printComma) {
				os << ", ";
			}
			printComma = true; // print comma after first
			os << ident << " = ";
			fields.at(ident)->print(os);
		}
		os << " }";
	}

	Expr* copy() const override {
		std::vector<Field> fieldsCopy;
		for (const std::string& ident : idents) {
			fieldsCopy.push_back({ ident, fields.at(ident)->copy() });
		}
		return new ERecordLit(loc, fieldsCopy, type);
	}

	Expr* subst(const std::string& subIdent, const Expr* subExpr) const override {
		std::vector<Field> fieldsCopy;
		for (const std::string& ident : idents) {
			fieldsCopy.push_back({ ident, fields.at(ident)->subst(subIdent, subExpr) });
		}
		return new ERecordLit(loc, fieldsCopy, type);
	}

	Value* eval() const override {
		// TODO: implement
		// add VRecord?
		return nullptr;
	}

	const Type* type_syn(const Context<const Type*>& typeCtx, bool reportErrors = true) const override {
		/*if (!typeAnn) {
		    if (reportErrors) {
		        report_error_at_expr("failed to synthesize record type");
		    }
		    return nullptr;
		}
		if (!type_ana(typeAnn, typeCtx)) {
		    if (reportErrors) {
		        std::ostringstream oss;
		        oss << "expression does not analyze against type " << typeAnn;
		        report_error_at_expr(oss.str());
		    }
		    return nullptr;
		}
		return typeAnn;*/
		// TODO
		return nullptr;
	}

	bool type_ana(const Type* type, const Context<const Type*>& typeCtx) const override {
		const TRecord* recordType = type->as<TRecord>();
		if (!recordType) { return false; }
		if (recordType->fields.size() != fields.size()) { return false; }
		for (auto& recordField : recordType->fields) {
			auto it = fields.find(recordField.first);
			if (it == fields.end()) { return false; }
			if (!it->second->type_ana(recordField.second, typeCtx)) { return false; }
		}
		return true;
	}

private:
	std::vector<std::string> idents; // original order of idents
};
