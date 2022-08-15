alc: main.cpp file.hpp lexer.hpp location.hpp parser.hpp
	g++ -std=c++17 main.cpp -o alc

.PHONY: clean
clean:
	rm alc