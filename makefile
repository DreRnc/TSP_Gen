# Compiler
CXX = g++
# Compiler flags
CXXFLAGS = -std=c++20 -O3

all: bin/tspg_seq bin/tspg_par_native bin/tspg_par_native_dyn bin/tspg_par_ff

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

obj/tspg_seq.o: tspg_seq.cpp src/distancefuncs.hpp src/geneticfuncs.hpp src/utilfuncs.hpp utils/argumentparser.hpp utils/genetictimer.hpp utils/utimer.hpp
	mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/tspg_par_native.o: tspg_par_native.cpp src/distancefuncs.hpp src/geneticfuncs.hpp src/utilfuncs.hpp utils/argumentparser.hpp utils/genetictimer.hpp utils/utimer.hpp src/utilfuncs.hpp
	mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/tspg_par_native_dyn.o: tspg_par_native_dyn.cpp src/distancefuncs.hpp src/geneticfuncs.hpp src/utilfuncs.hpp utils/argumentparser.hpp utils/genetictimer.hpp utils/utimer.hpp src/utilfuncs.hpp
	mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/tspg_par_ff.o: tspg_par_ff.cpp src/distancefuncs.hpp src/geneticfuncs.hpp src/utilfuncs.hpp utils/argumentparser.hpp utils/genetictimer.hpp utils/utimer.hpp src/utilfuncs.hpp
	mkdir -p obj
	$(CXX) $(CXXFLAGS) -c $< -I/usr/local/include -L/usr/local/libs -lfastflow -o $@

# Linking
bin/tspg_seq: obj/tspg_seq.o obj/distancefuncs.o obj/geneticfuncs.o obj/utilfuncs.o
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -o $@ -pthread

bin/tspg_par_native: obj/tspg_par_native.o obj/distancefuncs.o obj/geneticfuncs.o obj/utilfuncs.o
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -o $@ -pthread

bin/tspg_par_native_dyn: obj/tspg_par_native_dyn.o obj/distancefuncs.o obj/geneticfuncs.o obj/utilfuncs.o
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -o $@ -pthread

bin/tspg_par_ff: obj/tspg_par_ff.o obj/distancefuncs.o obj/geneticfuncs.o obj/utilfuncs.o
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $^ -o $@ -pthread

# Clean
clean:
	rm -rf obj bin

.PHONY: all clean
