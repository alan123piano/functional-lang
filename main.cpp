#include <deque>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"
#include "source.hpp"
#include "evaluator.hpp"

// later, Normal mode will change to a compilation mode
enum Mode { Normal, Lex, Parse };

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "No input file provided" << std::endl;
		return 1;
	}
	if (argc >= 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
		std::cout << "Usage: alc file [--lex|--parse]" << std::endl;
		return 0;
	}

	std::ifstream ifs(argv[1]);
	if (!ifs) {
		std::cout << "Invalid file path: " << argv[1] << std::endl;
		return 1;
	}

	Mode mode = Mode::Normal;
	if (argc >= 3) {
		if (!strcmp(argv[2], "--lex")) {
			mode = Mode::Lex;
		} else if (!strcmp(argv[2], "--parse")) {
			mode = Mode::Parse;
		}
	}

	// echo cl args
	std::cout << "alc";
	for (int i = 1; i < argc; ++i) {
		std::cout << " " << argv[i];
	}
	std::cout << std::endl;

	Source source(ifs, argv[1]);
	Lexer lexer(source);
	std::deque<Token> tokens = lexer.get_tokens();
	if (mode == Mode::Lex) {
		for (Token token : tokens) {
			std::cout << token << ' ';
		}
		std::cout << std::endl;
		return 0;
	}
	Parser parser(source, std::move(tokens));
	Expr* ast = parser.parse();
	if (mode == Mode::Parse) {
		if (source.has_errors()) {
			source.emit_errors(std::cout);
		} else {
			std::cout << ast << std::endl;
		}
		return 0;
	}
	if (source.has_errors()) {
		source.emit_errors(std::cout);
		return 1;
	} else {
		if (!ast) {
			throw std::runtime_error("Received invalid AST without emitting errors");
		}
		Evaluator evaluator(source);
		Value* value = evaluator.eval(ast);
		if (source.has_errors()) {
			source.emit_errors(std::cout);
			return 1;
		} else {
			if (!value) {
				throw std::runtime_error("Received invalid value without emitting errors");
			}
			std::cout << value << std::endl;
			return 0;
		}
	}
}
