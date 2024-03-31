# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++20 -O3

all: bin/tspg_seq

# Compiling source files
obj/distancefuncs.o: src/distancefuncs.cpp src/distancefuncs.hpp
	mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/geneticfuncs.o: src/geneticfuncs.cpp src/geneticfuncs.hpp src/utilfuncs.hpp
	mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/utilfuncs.o: src/utilfuncs.cpp src/utilfuncs.hpp
	mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/tspg_seq.o: tspg_seq.cpp src/distancefuncs.hpp src/geneticfuncs.hpp utils/argumentparser.hpp utils/genetictimer.hpp utils/utimer.hpp
	mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Linking
bin/tspg_seq: obj/tspg_seq.o obj/distancefuncs.o obj/geneticfuncs.o obj/utilfuncs.o
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -o $@

# Clean
clean:
	rm -rf obj bin

.PHONY: all clean