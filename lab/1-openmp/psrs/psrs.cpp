#include <algorithm>
#include <chrono>
#include <iostream>
#include <omp.h>
#include <random>

using std::copy;
using std::cout;
using std::endl;
using std::equal;
using std::fill;
using std::max;
using std::mt19937;
using std::ofstream;
using std::random_device;
using std::sort;
using std::uniform_int_distribution;

using ns = std::chrono::nanoseconds;
auto now = std::chrono::high_resolution_clock::now;

const size_t DEFAULT_THREADS = 3;

const int UPPER_BOUND = 1000000;
const int LOWER_BOUND = 0;

const int DEFAULT_ARRAY[] = {15, 46, 48, 93, 39, 6,  72, 91, 14,
                             36, 69, 40, 89, 61, 97, 12, 21, 54,
                             53, 97, 84, 58, 32, 27, 33, 72, 20};

/* Parallel Sorting by Regular Sampling (PSRS)
 *   - `array`:   The array to be sorted,
 *   - `size`:    The size of `array`
 *   - `threads`: Number of threads
 */
void psrs(int array[], unsigned int size, unsigned int threads)
{
    if (threads > size)
        threads = size;
    // Maximize load balance
    unsigned int size_per_thread = (size + threads - 1) / threads;
    // In case the last thread gets no/negtive work
    if (size_per_thread * (threads - 1) > size) {
        size_per_thread--;
    }

    int samples[threads * threads];
    int pivots[threads - 1];

    unsigned int split_sizes[threads][threads];
    int         *split_arrays[threads][threads];

    unsigned int subsize_after_swap[threads];

    unsigned int max_split_size =
        max(size_per_thread, size - size_per_thread * (threads - 1));
    for (unsigned int i = 0; i < threads; i++) {
        for (unsigned int j = 0; j < threads; j++) {
            split_arrays[i][j] = new int[max_split_size];
        }
    }

    omp_set_num_threads(threads);
#pragma omp parallel
    {
        /* 1: Uniform partition */
        int  id       = omp_get_thread_num();
        int *subarray = array + id * size_per_thread;
        int  subsize  = size_per_thread;

        if (id == threads - 1) {
            subsize = size - id * size_per_thread;
        }

        /* 2: Local sorting */
        sort(subarray, subarray + subsize);

        /* 3: Regular sampling */
        for (int i = 0; i < threads; i++) {
            samples[id * threads + i] = subarray[subsize * i / threads];
        }

#pragma omp barrier
#pragma omp master
        {
            /* 4: Sample sorting */
            sort(samples, samples + threads * threads);
            /* 5: Pivot selection */
            for (int i = 1; i < threads; i++) {
                pivots[i - 1] = samples[i * threads];
            }
        }

#pragma omp barrier
        /* 6: Partition */
        unsigned int index = 0;
        fill(split_sizes[id], split_sizes[id] + threads, 0);
        for (int i = 0; i < subsize; i++) {
            while (index < threads - 1 && subarray[i] > pivots[index])
                index++;
            split_arrays[id][index][split_sizes[id][index]++] = subarray[i];
        }

#pragma omp barrier
        /* 7: Total exchange */
        // Calculating new subarray and subsize
        subsize_after_swap[id] = 0;
        for (int i = 0; i < threads; i++) {
            subsize_after_swap[id] += split_sizes[i][id];
        }
        subsize = subsize_after_swap[id];
#pragma omp barrier
        subarray = array;
        for (int i = 0; i < id; i++) {
            subarray += subsize_after_swap[i];
        }

        /* 8: Local merging */
        int left_indices[threads];
        fill(left_indices, left_indices + threads, 0);
        for (int i = 0; i < subsize; i++) {
            int min_index = -1;
            int min_num   = 0x7fffffff;
            for (int j = 0; j < threads; j++) {
                if (left_indices[j] < split_sizes[j][id]
                    && split_arrays[j][id][left_indices[j]] < min_num)
                {
                    min_index = j;
                    min_num   = split_arrays[j][id][left_indices[j]];
                }
            }
            subarray[i] = min_num;
            left_indices[min_index]++;
        }
    }

    for (unsigned int i = 0; i < threads; i++) {
        for (unsigned int j = 0; j < threads; j++) {
            delete split_arrays[i][j];
        }
    }
}

int *generate_random_array(unsigned int size)
{
    int          *array = new int[size];
    random_device rd;
    mt19937       gen(rd());

    uniform_int_distribution<int> dis(LOWER_BOUND, UPPER_BOUND);

    for (unsigned int i = 0; i < size; i++) {
        array[i] = dis(gen);
    }

    return array;
}

int main(int argc, char *argv[])
{
    int  threads = DEFAULT_THREADS;
    int  size    = 27;
    int *array   = new int[size];
    copy(DEFAULT_ARRAY, DEFAULT_ARRAY + size, array);

    if (argc > 1) {
        threads = atoi(argv[1]);
#ifdef DEBUG
        cout << "[Debug] Set thread number to " << threads << "." << endl;
#endif
    }

    if (argc > 2) {
        delete[] array;
        size  = atoi(argv[2]);
        array = generate_random_array(size);
#ifdef DEBUG
        cout << "[Debug] Set array size to " << size << "." << endl;
#endif
    }

    int *array_copy = new int[size];
    copy(array, array + size, array_copy);

    auto psrs_start = now();
    psrs(array, size, threads);
    auto  psrs_end  = now();
    ns    psrs_ns   = psrs_end - psrs_start;
    float psrs_time = psrs_ns.count() / 1e6;

    auto std_start = now();
    sort(array_copy, array_copy + size);
    auto  std_end  = now();
    ns    std_ns   = std_end - std_start;
    float std_time = std_ns.count() / 1e6;

    float speedup = std_time / psrs_time;

    if (!equal(array, array + size, array_copy)) {
        cout << "[Error] PSRS result is wrong." << endl;
    }

    cout << "[Info] Time used by psrs():      " << psrs_time << " ms" << endl
         << "[Info] Time used by std::sort(): " << std_time << " ms" << endl
         << "[Info] Speedup: " << speedup << endl;

#ifdef DEBUG
    cout << "[Debug] PSRS result: ";
    for (int i = 0; i < size; i++) {
        cout << array[i] << " ";
    }
    cout << endl;
#endif

    delete[] array;
    delete[] array_copy;

    return 0;
}
