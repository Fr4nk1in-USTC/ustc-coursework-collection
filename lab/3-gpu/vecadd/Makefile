CXX = nvcc
CXXFLAGS = -O0

all: vecadd

vecadd: vecadd.cu
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f vecadd

.PHONY: all clean
