#include <deque>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include "Type.h"
#include "Lexer.h"
#include "Parser.h"
#include "Source.h"
#include "Context.h"

/**
 * File input: reads from file
 * Repl input: reads from cin
 */
enum class InputMode { File, Repl };

// compilation mode will be added later
enum class OutputMode { Eval, Lex, Parse, Type };

int run(std::istream& is, const std::string& filepath, OutputMode outputMode) {
	// initialize source
	Source source(is, filepath);

	// lex
	Lexer lexer(source);
	std::deque<Token> tokens = lexer.get_tokens();
	if (source.has_errors()) {
		source.emit_errors(std::cout);
		return 1;
	}
	if (outputMode == OutputMode::Lex) {
		for (Token token : tokens) {
			std::cout << token << ' ';
		}
		std::cout << std::endl;
		return 0;
	}

	// parse
	Parser parser(std::move(tokens));
	Expr* ast = parser.parse();
	if (source.has_errors()) {
		source.emit_errors(std::cout);
		return 1;
	}
	if (!ast) {
		throw std::runtime_error("Received invalid AST without emitting errors");
	}
	if (outputMode == OutputMode::Parse) {
		std::cout << ast << std::endl;
		return 0;
	}

	// type-check
	const Type* type = ast->type_syn(Context<const Type*>());
	if (source.has_errors()) {
		source.emit_errors(std::cout);
		return 1;
	}
	if (!type) {
		throw std::runtime_error("Failed to synthesize type without reporting errors");
	}
	if (outputMode == OutputMode::Type) {
		std::cout << type << std::endl;
		return 0;
	}

	// evaluate
	Value* value = ast->eval();
	if (source.has_errors()) {
		source.emit_errors(std::cout);
		return 1;
	}
	if (!value) {
		throw std::runtime_error("Received invalid value without emitting errors");
	}
	std::cout << value << " : " << value->get_type() << std::endl;
	return 0;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "No input file provided" << std::endl;
		return 1;
	}
	if (argc >= 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
		std::cout << "Usage: alc file|--repl [--lex|--parse|--type]" << std::endl;
		return 0;
	}

	InputMode inputMode = !strcmp(argv[1], "--repl")
	                      ? InputMode::Repl
	                      : InputMode::File;
	std::ifstream ifs(argv[1]);
	std::istream& is = inputMode == InputMode::Repl
	                   ? std::cin
	                   : ifs;
	if (!is) {
		std::cout << "Invalid file path: " << argv[1] << std::endl;
		return 1;
	}

	OutputMode outputMode = OutputMode::Eval;
	if (argc >= 3) {
		if (!strcmp(argv[2], "--lex")) {
			outputMode = OutputMode::Lex;
		} else if (!strcmp(argv[2], "--parse")) {
			outputMode = OutputMode::Parse;
		} else if (!strcmp(argv[2], "--type")) {
			outputMode = OutputMode::Type;
		}
	}

	// echo cl args
	std::cout << "alc";
	for (int i = 1; i < argc; ++i) {
		std::cout << " " << argv[i];
	}
	std::cout << std::endl;

	if (inputMode == InputMode::Repl) {
		std::stringstream ss;
		std::string input;
		while (std::getline(is, input)) {
			if (input.empty()) {
				std::cout << "\x1b[A"; // go up a line
				run(ss, "", outputMode);
				ss = std::stringstream();
			} else {
				ss << input << "\n";
			}
		}
	} else {
		return run(is, argv[1], outputMode);
	}
}
