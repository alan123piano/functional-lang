#include "Parser.h"

Expr* Parser::parse_expr(int minBindingPower, bool reportErrors) {
	Token peek = tokens.front();
	Expr* lhs = nullptr;
	switch (peek.type) {
	case TokenType::IntLit: {
		// IntLit
		tokens.pop_front();
		long long value;
		try {
			value = std::stoll(peek.value);
		} catch (std::exception&) {
			peek.report_error_at_token("invalid int literal");
			return nullptr;
		}
		lhs = new EIntLit(peek.loc, nullptr, value);
		break;
	}
	case TokenType::FloatLit: {
		// FloatLit
		tokens.pop_front();
		double value;
		try {
			value = std::stod(peek.value);
		} catch (std::exception&) {
			peek.report_error_at_token("invalid double literal");
			return nullptr;
		}
		lhs = new EFloatLit(peek.loc, nullptr, value);
		break;
	}
	case TokenType::True: {
		// BoolLit(true)
		tokens.pop_front();
		lhs = new EBoolLit(peek.loc, nullptr, true);
		break;
	}
	case TokenType::False: {
		// BoolLit(false)
		tokens.pop_front();
		lhs = new EBoolLit(peek.loc, nullptr, false);
		break;
	}
	case TokenType::Ident: {
		// Ident
		tokens.pop_front();
		lhs = new EVar(peek.loc, nullptr, peek.value);
		break;
	}
	case TokenType::LeftParen: {
		// '(' <Expr> ')' or <EUnitLit>
		tokens.pop_front();
		if (tokens.front().type == TokenType::RightParen) {
			// <EUnitLit>
			tokens.pop_front();
			lhs = new EUnitLit(peek.loc, Type::Unit());
		} else {
			// '(' <Expr> ')'
			lhs = parse_expr();
			if (!lhs) { return nullptr; }
			if (!expect_token(TokenType::RightParen)) { return nullptr; }
		}
		break;
	}
	case TokenType::LeftBrace: {
		// <ERecordLit>
		std::vector<ERecordLit::Field> fields;
		std::unordered_set<std::string> identsUsed; // prevent duplicate idents
		// parse fields
		do {
			// on the first loop, this will pop the LeftBrace;
			// on subsequent loops, it will pop the comma
			// (kinda hacky, but reduces code duplication)
			tokens.pop_front();
			std::optional<Token> ident = expect_token(TokenType::Ident);
			if (!ident) { return nullptr; }
			if (!expect_token(TokenType::Equals)) { return nullptr; }
			Expr* expr = parse_expr();
			if (!expr) { return nullptr; }
			if (!identsUsed.insert(ident->value).second) {
				peek.report_error_at_token("duplicate record field '" + ident->value + "'");
				return nullptr;
			}
			fields.push_back({ ident->value, expr });
		} while (tokens.front().type == TokenType::Comma);
		if (!expect_token(TokenType::RightBrace)) { return nullptr; }
		lhs = new ERecordLit(peek.loc, nullptr, fields);
		// match type based on idents
		for (auto& entry : typeTable) {
			const TRecord* recordType = entry.second->as<TRecord>();
			if (!recordType) { continue; }
			if (recordType->fields.size() != fields.size()) { continue; }
			bool isMatch = true;
			for (auto& field : fields) {
				if (recordType->fields.find(field.ident) == recordType->fields.end()) {
					isMatch = false;
					break;
				}
			}
			if (!isMatch) { continue; }
			lhs->typeAnn = recordType;
		}
		if (!lhs->typeAnn) {
			peek.report_error_at_token("unable to match record type for identifier set");
			return nullptr;
		}
		break;
	}
	case TokenType::Let: {
		// <ELet>
		tokens.pop_front();
		EVar* varExpr = parse_ident();
		if (!varExpr) { return nullptr; }
		if (!expect_token(TokenType::Equals)) { return nullptr; }
		Expr* value = parse_expr();
		if (!value) { return nullptr; }
		if (!expect_token(TokenType::In)) { return nullptr; }
		Expr* body = parse_expr();
		if (!body) { return nullptr; }
		lhs = new ELet(peek.loc, nullptr, varExpr, value, body);
		break;
	}
	case TokenType::If: {
		// <EIf>
		tokens.pop_front();
		Expr* test = parse_expr();
		if (!test) { return nullptr; }
		if (!expect_token(TokenType::Then)) { return nullptr; }
		Expr* body = parse_expr();
		if (!body) { return nullptr; }
		if (!expect_token(TokenType::Else)) { return nullptr; }
		Expr* elseBody = parse_expr();
		if (!elseBody) { return nullptr; }
		lhs = new EIf(peek.loc, nullptr, test, body, elseBody);
		break;
	}
	case TokenType::Fun: {
		// <EFun>
		tokens.pop_front();
		EVar* varExpr = parse_ident();
		if (!varExpr) { return nullptr; }
		if (!expect_token(TokenType::Arrow)) { return nullptr; }
		Expr* body = parse_expr();
		if (!body) { return nullptr; }
		lhs = new EFun(peek.loc, nullptr, varExpr, body);
		break;
	}
	case TokenType::Fix: {
		// <EFix>
		tokens.pop_front();
		EVar* varExpr = parse_ident();
		if (!varExpr) { return nullptr; }
		if (!expect_token(TokenType::Arrow)) { return nullptr; }
		Expr* body = parse_expr();
		if (!body) { return nullptr; }
		lhs = new EFix(peek.loc, nullptr, varExpr, body);
		break;
	}
	case TokenType::Minus: {
		// '-' <Expr>
		tokens.pop_front();
		Expr* right = parse_expr();
		if (!right) { return nullptr; }
		lhs = new EUnaryOp(peek.loc, nullptr, peek, right);
		break;
	}
	case TokenType::Not: {
		// '!' <Expr>
		tokens.pop_front();
		Expr* right = parse_expr();
		if (!right) { return nullptr; }
		lhs = new EUnaryOp(peek.loc, nullptr, peek, right);
		break;
	}
	default: {
		// unexpected token
		if (reportErrors) {
			std::ostringstream oss;
			oss << "expected expression; got token '" << peek.type << "'";
			peek.report_error_at_token(oss.str());
		}
		return nullptr;
	}
	}
	// handle recursive cases with Pratt parsing
	while (true) {
		peek = tokens.front();
		bool matched = false;
		switch (peek.type) {
		case TokenType::Mul:
		case TokenType::Div:
		case TokenType::Mod:
		case TokenType::Plus:
		case TokenType::Minus:
		case TokenType::Equals:
		case TokenType::NotEquals:
		case TokenType::Lt:
		case TokenType::Gt:
		case TokenType::Leq:
		case TokenType::Geq:
		case TokenType::And:
		case TokenType::Or: {
			// handle <EBinaryOp>
			BindingPower bindingPower = BindingPower::BinOp(peek);
			if (bindingPower.left < minBindingPower) { break; }
			tokens.pop_front();
			Expr* rhs = parse_expr(bindingPower.right);
			if (!rhs) { break; }
			matched = true;
			lhs = new EBinaryOp(lhs->loc, nullptr, lhs, peek, rhs);
			break;
		}
		default: {
			// handle <EFunAp>
			BindingPower bindingPower = BindingPower::FunAp();
			if (bindingPower.left < minBindingPower) { break; }
			// it's a little hacky to call parse_expr to check if we can create a EFunAp..
			// this trick relies on parse_expr not chomping tokens if it returns a nullptr
			Expr* rhs = parse_expr(bindingPower.right, false);
			if (!rhs) { break; }
			matched = true;
			lhs = new EFunAp(lhs->loc, nullptr, lhs, rhs);
			break;
		}
		}
		if (!matched) { break; }
	}
	// check for type annotation
	if (tokens.front().type == TokenType::Colon) {
		tokens.pop_front();
		const Type* typeAnn = parse_type_expr();
		if (!typeAnn) { return nullptr; }
		lhs->typeAnn = typeAnn;
	}
	return lhs;
}

EVar* Parser::parse_ident() {
	Expr* expr = parse_expr(std::numeric_limits<int>::max());
	if (!expr) { return nullptr; }
	EVar* var = dynamic_cast<EVar*>(expr);
	if (!var) {
		expr->report_error_at_expr("expected identifier expression");
		return nullptr;
	}
	return var;
}

const Type* Parser::parse_type_expr(bool reportErrors) {
	Token peek;
	std::vector<const Type*> types;
	while (true) {
		peek = tokens.front();
		if (peek.type == TokenType::Ident) {
			tokens.pop_front();
			auto it = typeTable.find(peek.value);
			if (it == typeTable.end()) {
				peek.report_error_at_token("unbound typename '" + peek.value + "'");
				return nullptr;
			}
			types.push_back(it->second);
		} else if (peek.type == TokenType::LeftParen) {
			tokens.pop_front();
			const Type* inner = parse_type_expr();
			if (!inner) { return nullptr; }
			types.push_back(inner);
			if (!expect_token(TokenType::RightParen)) { return nullptr; }
		} else {
			break;
		}
		// keep chomping '*' for tuple types
		if (tokens.front().type != TokenType::Mul) { break; }
		tokens.pop_front();
	}
	// convert parsed types vector into single type or tuple, or report error
	const Type* lhs;
	if (types.size() == 0) {
		if (reportErrors) {
			std::ostringstream oss;
			oss << "expected type identifier; got token '" << peek.type << "'";
			peek.report_error_at_token(oss.str());
		}
		return nullptr;
	} else if (types.size() == 1) {
		lhs = types[0];
	} else {
		lhs = new TTuple(std::move(types));
	}
	// parse arrow type
	if (tokens.front().type == TokenType::Arrow) {
		tokens.pop_front();
		const Type* rhs = parse_type_expr();
		if (!rhs) { return nullptr; }
		lhs = new TArrow(lhs, rhs);
	}
	return lhs;
}

std::optional<std::pair<std::string, Type*>> Parser::parse_type_decl() {
	if (!expect_token(TokenType::Type)) { return std::nullopt; }
	std::optional<Token> ident = expect_token(TokenType::Ident);
	if (!ident) { return std::nullopt; }
	std::string typeName = ident->value;
	// check for duplicate type name
	if (typeTable.find(typeName) != typeTable.end()) {
		std::ostringstream oss;
		oss << "duplicate type name '" << typeName << "'";
		ident->report_error_at_token(oss.str());
		return std::nullopt;
	}
	if (!expect_token(TokenType::Equals)) { return std::nullopt; }
	Token peek = tokens.front();
	// FIRST set of VariantDecl: '|', Ident
	// FIRST set of RecordDecl: '{'
	switch (peek.type) {
	case TokenType::Bar: {
		// VariantDecl with '|' in front
		tokens.pop_front();
		// no break - goto VariantDecl ident case
	}
	case TokenType::Ident: {
		// VariantDecl
		// expect token because case TokenType::Bar leads in to here
		if (!expect_token(TokenType::Ident)) { return std::nullopt; }
		std::vector<TVariant::Case> cases;
		// parse first case
		const Type* type = parse_type_expr(false);
		cases.push_back({ peek.value, type });
		// parse additional cases
		while (true) {
			peek = tokens.front();
			if (peek.type != TokenType::Bar) { break; }
			tokens.pop_front();
			std::optional<Token> ident = expect_token(TokenType::Ident);
			if (!ident) { return std::nullopt; }
			const Type* type = parse_type_expr(false);
			cases.push_back({ ident->value, type });
		}
		if (!expect_token(TokenType::Semicolon)) { return std::nullopt; }
		return std::make_pair(typeName, new TVariant(typeName, std::move(cases)));
	}
	case TokenType::LeftBrace: {
		// RecordDecl
		tokens.pop_front();
		std::vector<TRecord::Field> fields;
		std::unordered_set<std::string> identsUsed; // prevent duplicate idents
		// parse cases
		bool expectingComma = false;
		while (true) {
			if (expectingComma) {
				peek = tokens.front();
				if (peek.type != TokenType::Comma) { break; }
				tokens.pop_front();
			}
			expectingComma = true; // expect comma to come before every case after the first
			std::optional<Token> ident = expect_token(TokenType::Ident);
			if (!ident) { return std::nullopt; }
			if (!expect_token(TokenType::Colon)) { return std::nullopt; }
			const Type* type = parse_type_expr();
			if (!type) {
				peek = tokens.front();
				std::ostringstream oss;
				oss << "expected type expression; got token '" << peek << "'";
				peek.report_error_at_token(oss.str());
				return std::nullopt;
			}
			// check for duplicate field
			if (!identsUsed.insert(ident->value).second) {
				ident->report_error_at_token("duplicate record field '" + ident->value + "'");
				return std::nullopt;
			}
			fields.push_back({ ident->value, type });
		}
		if (!expect_token(TokenType::RightBrace)) { return std::nullopt; }
		if (!expect_token(TokenType::Semicolon)) { return std::nullopt; }
		return std::make_pair(typeName, new TRecord(typeName, std::move(fields)));
	}
	default: {
		std::ostringstream oss;
		oss << "expected type declaration; got token '" << peek << "'";
		peek.report_error_at_token(oss.str());
		return std::nullopt;
	}
	}
}
