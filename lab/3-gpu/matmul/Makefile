CXX = nvcc
CXXFLAGS = -O0

all: matmul

matmul: matmul.cu
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f matmul

.PHONY: all clean
