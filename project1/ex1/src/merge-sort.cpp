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

#define TIME_FILE     OUTPUT_DIR "merge_sort/time.txt"
#define OUTPUT_PREFIX OUTPUT_DIR "merge_sort/result_"
#define OUTPUT_SUFFIX ".txt"

// mid: the starting index of second array
void merge(int *array, int length, int mid);
void mergesort(int *array, int length);

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
        mergesort(arrays[i], 1 << exps[i]);
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

/** Merge two sorted arrays in a continuous memory space
 * @param array  The starting address of arrays to be merged
 * @param length The length of the merged array
 * @param mid    The starting index of second array
 */
void merge(int *array, int length, int mid)
{
    int *left  = new int[mid + 1];
    int *right = new int[length - mid + 1];
    copy(array, array + mid, left);
    copy(array + mid, array + length, right);
    left[mid]           = inf;
    right[length - mid] = inf;

    int l = 0, r = 0;
    for (int i = 0; i < length; ++i) {
        if (left[l] < right[r]) {
            array[i] = left[l];
            ++l;
        } else {
            array[i] = right[r];
            ++r;
        }
    }
    delete[] left;
    delete[] right;
}

/** Merge sort
 * @param array  The array to be sorted
 * @param length The length of the array
 */
void mergesort(int *array, int length)
{
    if (length < 2)
        return;
    int mid = length / 2;
    mergesort(array, mid);
    mergesort(array + mid, length - mid);
    merge(array, length, mid);
}
