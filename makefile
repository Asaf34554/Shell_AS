#!make -f

#CXX=clang++-9 
CXX=gcc
CXXFLAGS= -Wall -g

HEADERS=shell.h
OBJECTS=shell2.o shell2functions.o
SOURCES=shell2.c shell2functions.c

all: myshell
	./$^

mem_test: myshell
	valgrind ./$^

myshell: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o myshell

%.o: %.c $(HEADERS)
	$(CXX) $(CXXFLAGS) --compile $< -o $@

clean:
	rm -f *.o myshell