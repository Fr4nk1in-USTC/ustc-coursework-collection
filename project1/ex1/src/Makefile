CXX = clang++
CXXFLAGS = -O3 -Wall -Wextra

all: gen-input heap quick merge counting

gen-input: gen-input.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

heap: heap-sort.cpp
	$(CXX) $(CXXFLAGS) -o $@-sort $^

quick: quick-sort.cpp
	$(CXX) $(CXXFLAGS) -o $@-sort $^

merge: merge-sort.cpp
	$(CXX) $(CXXFLAGS) -o $@-sort $^

counting: counting-sort.cpp
	$(CXX) $(CXXFLAGS) -o $@-sort $^

clean:
	rm -f gen-input heap quick merge counting

.PHONY: all clean
