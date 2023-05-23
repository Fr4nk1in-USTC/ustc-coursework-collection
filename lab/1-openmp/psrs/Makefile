CXX = clang++
CXXFLAGS = -Ofast -fopenmp
DEBUG = -g -D DEBUG

all: psrs

psrs: psrs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

debug: psrs.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG) -o $@ $^

clean:
	rm -f psrs debug

.PHONY: all clean
