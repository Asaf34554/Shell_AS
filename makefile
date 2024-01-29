#!make -f

#CXX=clang++-9 
CXX=gcc
CXXFLAGS= -Wall -g

HEADERS=shell.h
OBJECTS=shell2.o 
SOURCES=shell2.c 

all: myshell
	./$^

mem_test: myshell
	valgrind --leak-check=full --show-leak-kinds=all ./$^

myshell: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o myshell

%.o: %.c $(HEADERS)
	$(CXX) $(CXXFLAGS) --compile $< -o $@

clean:
	rm -f *.o myshell