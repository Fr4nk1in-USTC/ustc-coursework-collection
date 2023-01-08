CXX = clang++
CFLAGS = -O3 -Wall -Wextra

all: gen-input johnson

gen-input: gen-input.cpp
	$(CXX) $(CFLAGS) -o $@ $^

johnson: johnson.cpp
	$(CXX) $(CFLAGS) -o $@ $^

clean:
	rm -f gen-input johnson

.PHONY: all clean
