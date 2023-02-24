#include "Parser.h"

Expr* Parser::parse_expr(int minBindingPower, bool expectedExpr) {
	Expr* lhs = nullptr;
	switch (peek().type) {
	case TokenType::Ident: {
		Token token = pop_token();
		lhs = new EVar(token.loc, token.value, nullptr);
		break;
	}
	case TokenType::IntLit: {
		Token token = pop_token();
		long long value;
		try {
			value = std::stoll(token.value);
		} catch (std::exception&) {
			token.report_error_at_token("invalid int literal");
			return nullptr;
		}
		lhs = new EIntLit(token.loc, value);
		break;
	}
	case TokenType::FloatLit: {
		Token token = pop_token();
		double value;
		try {
			value = std::stod(token.value);
		} catch (std::exception&) {
			token.report_error_at_token("invalid double literal");
			return nullptr;
		}
		lhs = new EFloatLit(token.loc, value);
		break;
	}
	case TokenType::True: {
		Token token = pop_token();
		lhs = new EBoolLit(token.loc, true);
		break;
	}
	case TokenType::False: {
		Token token = pop_token();
		lhs = new EBoolLit(token.loc, false);
		break;
	}
	case TokenType::LeftParen: {
		// <EUnitLit>, '(' <Expr> ')', or <ETupleLit>
		Token leftParen = pop_token();
		if (peek().type == TokenType::RightParen) {
			// <EUnitLit>
			pop_token();
			lhs = new EUnitLit(leftParen.loc);
		} else {
			// '(' <Expr> ')' or <ETupleLit>
			std::vector<Expr*> fields;
			while (true) {
				Expr* expr = parse_expr();
				if (!expr) { break; }
				fields.push_back(expr);
				if (peek().type != TokenType::Comma) { break; }
			}
			if (!expect_token(TokenType::RightParen)) { return nullptr; }
			if (fields.empty()) {
				return nullptr;
			} else if (fields.size() == 1) {
				// '(' <Expr> ')'
				lhs = fields[0];
			} else {
				// <ETupleLit>
				lhs = new ETupleLit(leftParen.loc, std::move(fields));
			}
		}
		break;
	}
	case TokenType::LeftBrace: {
		// <ERecordLit>
		std::vector<ERecordLit::Field> fields;
		std::unordered_set<std::string> identsUsed; // prevent duplicate idents
		Token leftBrace = peek();
		// parse fields
		do {
			// on the first loop, this will pop the LeftBrace;
			// on subsequent loops, it will pop the comma
			// (kinda hacky, but reduces code duplication)
			pop_token();
			auto ident = expect_token(TokenType::Ident);
			if (!ident) { return nullptr; }
			if (!expect_token(TokenType::Equals)) { return nullptr; }
			Expr* expr = parse_expr();
			if (!expr) { return nullptr; }
			if (!identsUsed.insert(ident->value).second) {
				ident->report_error_at_token("duplicate record field '" + ident->value + "'");
				return nullptr;
			}
			fields.push_back({ ident->value, expr });
		} while (tokens.front().type == TokenType::Comma);
		if (!expect_token(TokenType::RightBrace)) { return nullptr; }
		// match type based on idents
		const Type* type = nullptr;
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
			type = recordType;
		}
		if (!type) {
			leftBrace.report_error_at_token("unable to match type to record literal from identifier set");
			return nullptr;
		}
		if (!expect_token(TokenType::RightBrace)) { return nullptr; }
		lhs = new ERecordLit(leftBrace.loc, std::move(fields), type);
		break;
	}
	case TokenType::Let: {
		// <ELet>
		Token let = pop_token();
		bool rec;
		if (peek().type == TokenType::Rec) {
			rec = true;
			pop_token();
		} else {
			rec = false;
		}
		EVar* var = parse_var();
		if (!var) { return nullptr; }
		if (!expect_token(TokenType::Equals)) { return nullptr; }
		Expr* value = parse_expr();
		if (!value) { return nullptr; }
		if (!expect_token(TokenType::In)) { return nullptr; }
		Expr* body = parse_expr();
		if (!body) { return nullptr; }
		lhs = new ELet(let.loc, rec, var, value, body);
		break;
	}
	case TokenType::If: {
		// <EIf>
		Token ifToken = pop_token();
		Expr* test = parse_expr();
		if (!test) { return nullptr; }
		if (!expect_token(TokenType::Then)) { return nullptr; }
		Expr* body = parse_expr();
		if (!body) { return nullptr; }
		if (!expect_token(TokenType::Else)) { return nullptr; }
		Expr* elseBody = parse_expr();
		if (!elseBody) { return nullptr; }
		lhs = new EIf(ifToken.loc, test, body, elseBody);
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
		if (expectedExpr) {
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
	return lhs;
}

EVar* Parser::parse_var() {
	switch (peek().type) {
	case TokenType::Ident: {
		Token ident = pop_token();
		return new EVar(ident.loc, ident.value);
	}
	case TokenType::LeftParen: {
		Token paren = pop_token();
		auto ident = expect_token(TokenType::Ident);
		if (!ident) { return nullptr; }
		expect_token(TokenType::Colon);
		const Type* typeAnn = parse_type_expr();
		if (!typeAnn) { return nullptr; }
		expect_token(TokenType::RightParen);
		return new EVar(paren.loc, ident->value, typeAnn);
	}
	default: {
		peek().report_error_at_token("expected variable expression");
		return nullptr;
	}
	}
}

const Type* parse_type_expr(bool expectedType) {
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
		if (expectedType) {
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
