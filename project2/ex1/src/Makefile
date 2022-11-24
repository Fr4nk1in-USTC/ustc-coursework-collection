CXX = clang++
CXXFLAGS = -O3 -Wall -Wextra

all: matrix-chain-order

matrix-chain-order: matrix-chain-order.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f matrix-chain-order

.PHONY:
	all clean
