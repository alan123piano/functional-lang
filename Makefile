alc: main.cpp evaluator.hpp expr.hpp lexer.hpp location.hpp parser.hpp scope.hpp source.hpp token.hpp value.hpp
	g++ -std=c++17 main.cpp -o alc

.PHONY: clean
clean:
	rm alc