CXX = clang++
CFLAGS = -O3 -Wall -Wextra

all: main gen-input

debug: CFLAGS += -g -DDEBUG
debug: main gen-input

main: main.cpp interval-tree.o
	$(CXX) $(CFLAGS) -o $@ $^

interval-tree.o: interval-tree.cpp
	$(CXX) $(CFLAGS) -c -o $@ $^

gen-input: gen-input.cpp
	$(CXX) $(CFLAGS) -o $@ $^

clean:
	rm -f main gen-input interval-tree.o

.PHONY: all debug clean
