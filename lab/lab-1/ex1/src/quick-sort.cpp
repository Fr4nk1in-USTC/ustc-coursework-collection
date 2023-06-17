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

#define TIME_FILE     OUTPUT_DIR "quick_sort/time.txt"
#define OUTPUT_PREFIX OUTPUT_DIR "quick_sort/result_"
#define OUTPUT_SUFFIX ".txt"

inline void swap(int &a, int &b)
{
    int temp = a;
    a        = b;
    b        = temp;
}

int  partition(int *array, int length);
void quick_sort(int *array, int length);

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
        quick_sort(arrays[i], 1 << exps[i]);
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

/** Partition the array into two parts
 * @param array  The array to be partitioned
 * @param length The length of the array
 * @return The index of the pivot
 */
int partition(int *array, int length)
{
    int pivot = array[length - 1];
    int i     = -1;
    for (int j = 0; j < length - 1; ++j) {
        if (array[j] < pivot) {
            ++i;
            swap(array[i], array[j]);
        }
    }
    swap(array[i + 1], array[length - 1]);
    return i + 1;
}

/** Quick sort
 * @param array  The array to be sorted
 * @param length The length of the array
 */
void quick_sort(int *array, int length)
{
    if (length <= 1)
        return;
    int pivot = partition(array, length);
    quick_sort(array, pivot);
    quick_sort(array + pivot + 1, length - pivot - 1);
}
