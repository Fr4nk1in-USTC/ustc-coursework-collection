#include <algorithm>
#include <iostream>
#include <mpi.h>
#include <random>

using std::copy;
using std::cout;
using std::endl;
using std::equal;
using std::fill;
using std::mt19937;
using std::ofstream;
using std::random_device;
using std::sort;
using std::uniform_int_distribution;

const int UPPER_BOUND = 1000000;
const int LOWER_BOUND = 0;

const size_t DEFAULT_SIZE = 27;

int default_array[] = {15, 46, 48, 93, 39, 6,  72, 91, 14, 36, 69, 40, 89, 61,
                       97, 12, 21, 54, 53, 97, 84, 58, 32, 27, 33, 72, 20};

void print_array(int *array, int size)
{
    for (int i = 0; i < size; i++) {
        cout << array[i] << " ";
    }
    cout << endl;
}

void psrs(int *array, int size, int *sorted)
{
    int id, num_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    /* 1: Scatter the input array to all processes */
    int  subarray_size = size / num_procs;
    int *subarray      = new int[subarray_size];
    MPI_Scatter(array, subarray_size, MPI_INT, subarray, subarray_size, MPI_INT,
                0, MPI_COMM_WORLD);

    /* 2: Local sorting */
    sort(subarray, subarray + subarray_size);

    /* 3: Regular sampling */
    // Get the samples
    int samples[num_procs];
    for (int i = 0; i < num_procs; i++) {
        samples[i] = subarray[i * subarray_size / num_procs];
    }
    // Gather all samples
    int sample_num = num_procs * num_procs;
    int sample_recv[sample_num];
    MPI_Gather(samples, num_procs, MPI_INT, sample_recv, num_procs, MPI_INT, 0,
               MPI_COMM_WORLD);

    /* 4: Sample sorting */
    if (id == 0)
        sort(sample_recv, sample_recv + sample_num);

    /* 5: Pivot selection */
    int pivots[num_procs - 1];
    if (id == 0) {
        for (int i = 1; i < num_procs; i++) {
            pivots[i - 1] = sample_recv[i * num_procs];
        }
    }
    MPI_Bcast(pivots, num_procs - 1, MPI_INT, 0, MPI_COMM_WORLD);

    /* 6: Partition */
    // We only store size and offset of each partition
    int partition_sizes[num_procs];
    int offsets[num_procs];
    int index  = 0;
    offsets[0] = 0;
    for (int i = 0; i < num_procs - 1; i++) {
        partition_sizes[i] = 0;
        while (index < subarray_size && subarray[index] <= pivots[i]) {
            partition_sizes[i]++;
            index++;
        }
        offsets[i + 1] = offsets[i] + partition_sizes[i];
    }
    partition_sizes[num_procs - 1] = subarray_size - offsets[num_procs - 1];

    /* 7: Total exchange */
    // First we get the size of each partition
    int partition_sizes_recv[num_procs];
    MPI_Alltoall(partition_sizes, 1, MPI_INT, partition_sizes_recv, 1, MPI_INT,
                 MPI_COMM_WORLD);
    // The we calculate the offset of each partition
    int new_subarray_size = 0;
    int offsets_recv[num_procs + 1];
    offsets_recv[0] = 0;
    for (int i = 0; i < num_procs; i++) {
        new_subarray_size  += partition_sizes_recv[i];
        offsets_recv[i + 1] = offsets_recv[i] + partition_sizes_recv[i];
    }
    // Then we create the new subarray and exchange elements
    int *new_subarray = new int[new_subarray_size];
    MPI_Alltoallv(subarray, partition_sizes, offsets, MPI_INT, new_subarray,
                  partition_sizes_recv, offsets_recv, MPI_INT, MPI_COMM_WORLD);

    /* 8: Local merging */
    int *merged = new int[new_subarray_size];
    int  left_indices[num_procs];
    copy(offsets_recv, offsets_recv + num_procs, left_indices);
    for (int i = 0; i < new_subarray_size; i++) {
        int min_index = -1;
        int min_value = UPPER_BOUND;
        for (int j = 0; j < num_procs; j++) {
            if (left_indices[j] < offsets_recv[j + 1]
                && new_subarray[left_indices[j]] < min_value)
            {
                min_index = j;
                min_value = new_subarray[left_indices[j]];
            }
        }
        merged[i] = min_value;
        left_indices[min_index]++;
    }

    /* 9: Gather the merged array */
    int size_each_proc[num_procs];
    MPI_Gather(&new_subarray_size, 1, MPI_INT, size_each_proc, 1, MPI_INT, 0,
               MPI_COMM_WORLD);

    int sorted_offsets[num_procs];
    if (id == 0) {
        sorted_offsets[0] = 0;
        for (int i = 1; i < num_procs; i++) {
            sorted_offsets[i] = sorted_offsets[i - 1] + size_each_proc[i - 1];
        }
    }

    MPI_Gatherv(merged, new_subarray_size, MPI_INT, sorted, size_each_proc,
                sorted_offsets, MPI_INT, 0, MPI_COMM_WORLD);

    /* Free the memory */
    delete[] subarray;
    delete[] new_subarray;
    delete[] merged;
}

int *random_array(const int num_procs, int &size)
{
#ifdef DEBUG
    cout << "[Debug] Generating random array." << endl;
    cout << "[Debug] The original size of input array is " << size << "."
         << endl;
#endif

    if (size % num_procs != 0) {
        size = (size / num_procs + 1) * num_procs;
        cout << "[Warning] The size of input array is not divisible by the "
                "number of processes."
             << endl
             << "[Warning] Expanding the size of input array to " << size << "."
             << endl;
    }

    int          *array = new int[size];
    random_device rd;
    mt19937       gen(rd());

    uniform_int_distribution<int> dis(LOWER_BOUND, UPPER_BOUND);

#ifdef DEBUG
    cout << "[Debug] Generating " << endl;
    cout.flush();
#endif
    for (int i = 0; i < size; i++) {
#ifdef DEBUG
        if (i % (size / 10) == 0) {
            cout << ".";
            cout.flush();
        }
#endif
        array[i] = dis(gen);
    }

#ifdef DEBUG
    cout << endl << "[Debug] Random array generated." << endl;
#endif
    return array;
}

int main(int argc, char *argv[])
{
    int id, num_procs;

    int *array  = nullptr;
    int *sorted = nullptr;
    int  size   = 0;

    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    /* Get the input array. If failed, stop all process */
    if (id == 0) {
        if (argc > 1) {
#ifdef DEBUG
            cout << "[Debug] Using random array." << endl;
#endif
            size   = atoi(argv[1]);
            array  = random_array(num_procs, size);
            sorted = new int[size];
        } else {
#ifdef DEBUG
            cout << "[Debug] Using default array." << endl;
#endif
            size = DEFAULT_SIZE;
            if (size % num_procs != 0) {
                size = (size / num_procs + 1) * num_procs;
                cout << "[Warning] The size of input array is not divisible by "
                        "the number of processes."
                     << endl
                     << "[Warning] Expanding the size of input array to "
                     << size << "." << endl;
            }
            array  = new int[size];
            sorted = new int[size];
            copy(default_array, default_array + size, array);

            for (int i = DEFAULT_SIZE; i < size; i++) {
                array[i] = UPPER_BOUND;
            }
        }
#ifdef DEBUG
        cout << "[Debug] The input array is: ";
        print_array(array, size);
#endif
        fill(sorted, sorted + size, 0);
    }

    MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);

#ifdef DEBUG
    if (id == 0) {
        cout << "[Debug] Starting psrs()." << endl;
    }
#endif
    double psrs_start = MPI_Wtime();
    psrs(array, size, sorted);
    double psrs_end  = MPI_Wtime();
    double psrs_time = psrs_end - psrs_start;
#ifdef DEBUG
    if (id == 0) {
        cout << "[Debug] psrs() done." << endl;
    }
#endif

    if (id == 0) {
#ifdef DEBUG
        cout << "[Debug] The sorted array is: ";
        print_array(sorted, size);
#endif
        double std_start = MPI_Wtime();
        sort(array, array + size);
        double std_end  = MPI_Wtime();
        double std_time = std_end - std_start;

        if (!equal(sorted, sorted + size, array)) {
            cout << "[Error] The result is incorrect." << endl;
        } else {
            cout << "[Info] The result is correct." << endl;
        }

        cout << "[Info] Time used by psrs():      " << psrs_time << " s."
             << endl
             << "[Info] Time used by std::sort(): " << std_time << " s." << endl
             << "[Info] Speedup: " << std_time / psrs_time << "." << endl;

        delete array;
        delete sorted;
    }

    MPI_Finalize();

    return 0;
}
