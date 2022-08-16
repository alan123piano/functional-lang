alc: main.cpp evaluator.hpp lexer.hpp location.hpp parser.hpp source.hpp
	g++ -std=c++17 main.cpp -o alc

.PHONY: clean
clean:
	rm alc