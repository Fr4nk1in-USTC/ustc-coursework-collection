CXX = clang++
CXXFLAGS = -O3 -Wall -Wextra

all: longest-common-subsequence

longest-common-subsequence: longest-common-subsequence.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f longest-common-subsequence

.PHONY:
	all clean
