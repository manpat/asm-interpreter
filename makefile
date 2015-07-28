SRC=$(shell find . -name "*.cpp")
OBJ=$(SRC:%.cpp=%.o)
CXXFLAGS=-std=c++14 -g -Wall

.PHONY: run

build: $(OBJ)
	@echo "--Linking--"
	@g++ $^ -obuild
	@echo "--Build complete--\n"

%.o: %.cpp %.h
	@echo "--Building "$*" module--"
	@g++ $(CXXFLAGS) -c $<

%.o: %.cpp
	@echo "--Building "$*" module--"
	@g++ $(CXXFLAGS) -c $<

run: build
	@echo "--Running--\n"
	@./build

clean:
	@rm *.o
	@echo "--Cleaned--\n"