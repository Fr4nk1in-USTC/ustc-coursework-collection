#include "config.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>

using std::copy;
using std::cout;
using std::endl;
using std::ifstream;
using std::ofstream;
using std::to_string;
using ns = std::chrono::nanoseconds;
auto now = std::chrono::high_resolution_clock::now;

#define TIME_FILE     OUTPUT_DIR "heap_sort/time.txt"
#define OUTPUT_PREFIX OUTPUT_DIR "heap_sort/result_"
#define OUTPUT_SUFFIX ".txt"

inline int parent(int i)
{
    return (i - 1) / 2;
}

inline int left(int i)
{
    return 2 * i + 1;
}

inline int right(int i)
{
    return 2 * i + 2;
}

inline void swap(int &a, int &b)
{
    int temp = a;
    a        = b;
    b        = temp;
}

void heapify(int *heap, int size, int index);
void build_heap(int *array, int length);
void heap_sort(int *array, int length);

int main()
{
    int      times[n_exps];
    int     *arrays[n_exps];
    ifstream fin(INPUT_FILE);
    ofstream fout[n_exps];
    ofstream time_file(TIME_FILE);

    // Initialize input and output files
    for (int i = 0; i < n_exps; ++i) {
        fout[i].open(OUTPUT_PREFIX + to_string(exps[i]) + OUTPUT_SUFFIX);
    }

    // Get numbers from input file
    int *data = new int[size];
    for (int i = 0; i < size; ++i) {
        fin >> data[i];
    }
    fin.close();
    for (int i = 0; i < n_exps; ++i) {
        int length = 1 << exps[i];
        arrays[i]  = new int[length];
        copy(data, data + length, arrays[i]);
    }
    delete[] data;

    // Sort numbers and get time of each scale
    for (int i = 0; i < n_exps; ++i) {
        auto start = now();
        heap_sort(arrays[i], 1 << exps[i]);
        auto end  = now();
        ns   time = end - start;
        times[i]  = time.count();
    }

    // Output the sorted arrays and times
    for (int i = 0; i < n_exps; ++i) {
        int length = 1 << exps[i];
        for (int j = 0; j < length; ++j)
            fout[i] << arrays[i][j] << endl;
        fout[i].close();
        time_file << exps[i] << ": " << times[i] << " ns" << endl;
    }
    time_file.close();

    // Print the time of each scale
    for (int i = 0; i < n_exps; ++i) {
        cout << "Exponent: " << exps[i] << ", time: " << times[i] << " ns"
             << endl;
    }

    // Print the sorted array of scale 2^3
    cout << endl << "Sorted array of scale 2^3:" << endl;
    for (int i = 0; i < 8; ++i)
        cout << arrays[0][i] << " ";
    cout << endl;
}

/** Heapify the subtree rooted at index
 * @param heap  The heap to be heapified
 * @param size  The size of the heap
 * @param index The index of the root of the subtree to be heapified
 */
void heapify(int *heap, int size, int index)
{
    int l, r, largest;
    while (index < size) {
        l       = left(index);
        r       = right(index);
        largest = index;

        if (l < size && heap[l] > heap[index])
            largest = l;

        if (r < size && heap[r] > heap[largest])
            largest = r;

        if (largest == index)
            break;

        swap(heap[index], heap[largest]);
        index = largest;
    }
}

/** Build a heap from an array
 * @param array  The array to be built into a heap
 * @param length The length of the array
 */
void build_heap(int *array, int length)
{
    for (int i = (length - 1) / 2; i >= 0; --i)
        heapify(array, length, i);
}

/** Heap sort
 * @param array  The array to be sorted
 * @param length The length of the array
 */
void heap_sort(int *array, int length)
{
    build_heap(array, length);
    for (int i = length - 1; i > 0; --i) {
        swap(array[0], array[i]);
        heapify(array, i, 0);
    }
}
