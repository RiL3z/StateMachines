# Simple makefile that starts off the project!
NFA: NFA.cpp
	g++ -g -pthread NFA.cpp -o NFA

test: Test.cpp
	g++ Test.cpp NFA.cpp -o test
