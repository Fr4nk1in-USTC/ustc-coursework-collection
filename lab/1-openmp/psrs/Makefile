CXX = g++
CXXFLAGS = -fopenmp
DEBUG = -g -D DEBUG

all: psrs

psrs: psrs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

debug: psrs.cpp
	$(CXX) $(CXXFLAGS) $(DEBUG) -o psrs $^

clean:
	rm -f psrs

.PHONY: all clean
