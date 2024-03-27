# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++20 -O3

all: tspg_seq

tspg_seq.o: tspg_seq.cpp utils/argument_parser.hpp utils/utimer.hpp
	$(CXX) -c $(CXXFLAGS) tspg_seq.cpp 

tspg_seq: tspg_seq.o
	$(CXX) tspg_seq.o -o tspg_seq

clean:
	rm -f *.o tspg_seq
