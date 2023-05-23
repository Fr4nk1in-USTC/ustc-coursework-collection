CXX = clang
CXXFLAGS = -fopenmp -Ofast

all: 1 2 3 4 5

1: 1-serialized.c
	$(CXX) $(CXXFLAGS) -o $@ $^

2: 2-parallel-region.c
	$(CXX) $(CXXFLAGS) -o $@ $^

3: 3-work-sharing-construct.c
	$(CXX) $(CXXFLAGS) -o $@ $^

4: 4-private-and-critical.c
	$(CXX) $(CXXFLAGS) -o $@ $^

5: 5-reduction.c
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f 1 2 3 4 5

.PHONY: all clean
