#include <deque>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include "file.hpp"
#include "lexer.hpp"
#include "parser.hpp"

int main(int argc, char* argv[]) {
	if (argc != 2) {
		std::cout << "No input file provided" << std::endl;
		return 1;
	}
	if (argc >= 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) {
		std::cout << "Usage: alc file" << std::endl;
		return 0;
	}

	std::ifstream ifs(argv[1]);
	File file(ifs, argv[1]);
	Lexer lexer(file);
	std::deque<Token> tokens = lexer.get_tokens();
	Parser parser(file, std::move(tokens));
	parser.parse()->print(std::cout);
	std::cout << std::endl;

	file.emit_errors(std::cout);
}
