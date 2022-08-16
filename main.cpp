#include <deque>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include "lexer.hpp"
#include "parser.hpp"
#include "source.hpp"

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "No input file provided" << std::endl;
		return 1;
	}
	if (argc >= 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
		std::cout << "Usage: alc file" << std::endl;
		return 0;
	}
	std::cout << "alc";
	for (int i = 1; i < argc; ++i) {
		std::cout << " " << argv[i];
	}
	std::cout << std::endl;

	std::ifstream ifs(argv[1]);
	Source source(ifs, argv[1]);
	Lexer lexer(source);
	std::deque<Token> tokens = lexer.get_tokens();
	Parser parser(source, std::move(tokens));
	parser.parse()->print(std::cout);
	std::cout << std::endl;

	source.emit_errors(std::cout);
}
