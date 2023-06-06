CXX = mpicc
CXXFLAGS = -Wall -Ofast

all: butterfly tree

butterfly: butterfly.c
	$(CXX) $(CXXFLAGS) -o $@ $^

tree: tree.c
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f butterfly tree

.PHONY: all clean
