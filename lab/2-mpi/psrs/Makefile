CXX = mpic++
CXXFLAGS = -Wall -Ofast
DEBUGFLAGS = -g -D DEBUG

all: psrs debug

psrs: psrs.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

debug: psrs.cpp
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -o $@ $^

clean:
	rm -f psrs debug

.PHONY: all clean
