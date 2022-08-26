PROJECT = alc
GCC = g++ -std=c++17
FLAGS = -Werror=all -Wno-sign-compare -Wno-parentheses
DIRS = * */* */*/*
SOURCES = $(wildcard *.cpp */*.cpp */*/*.cpp)
HEADERS = $(wildcard *.h */*.h */*/*.h *.hpp */*.hpp */*/*.hpp)
OBJECTS = $(patsubst %.cpp, %.o, $(SOURCES))

$(PROJECT): $(OBJECTS)
	$(GCC) $(FLAGS) -o $(PROJECT) $(OBJECTS)

%.o: %.cpp
	$(GCC) $(FLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(PROJECT) **/*.o
