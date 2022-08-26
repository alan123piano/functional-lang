alc: main.cpp evaluator.hpp expr.hpp lexer.hpp location.hpp parser.hpp scope.hpp source.hpp token.hpp type.hpp value.hpp
	g++ main.cpp -o alc -std=c++17 -Werror=all -Wno-sign-compare -Wno-parentheses

.PHONY: clean
clean:
	rm alc