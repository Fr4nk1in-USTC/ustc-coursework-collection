#include <algorithm>
#include <bits/chrono.h>
#include <chrono>
#include <cuda_runtime.h>
#include <iomanip>
#include <iostream>
#include <random>

using std::cout;
using std::endl;
using std::equal;
using std::fixed;
using std::mt19937;
using std::random_device;
using std::setprecision;
using std::setw;
using std::uniform_real_distribution;
using std::chrono::duration_cast;

using ns = std::chrono::nanoseconds;
auto now = std::chrono::high_resolution_clock::now;

const float  MIN_FLOAT = 0;
const float  MAX_FLOAT = 1000;
const size_t SCALES[]  = {100000, 200000, 1000000, 2000000, 10000000, 20000000};

const size_t NUM_THREADS = 256;

/**
 * Generate two random vectors `a` and `b` of size `size`.
 */
void generate_vector(float *a, float *b, size_t size)
{
    random_device                    rd;
    mt19937                          gen(rd());
    uniform_real_distribution<float> dis(MIN_FLOAT, MAX_FLOAT);

    for (size_t i = 0; i < size; i++) {
        a[i] = dis(gen);
        b[i] = dis(gen);
    }
}

/**
 * Add two vectors `a` and `b` and store the result in `c`.
 * The caller should ensure that `c` has the memory space.
 */
void vecadd_cpu(const float *a, const float *b, float *c, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        c[i] = a[i] + b[i];
    }
}

__global__ static void vecadd_kernel(const float *a, const float *b, float *c,
                                     size_t size)
{
    size_t index = blockDim.x * blockIdx.x + threadIdx.x;

    if (index < size) {
        c[index] = a[index] + b[index];
    }
}

bool verify(const float *a, const float *b, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (fabs(a[i] - b[i]) > 1e-5) {
            return false;
        }
    }
    return true;
}

int main()
{
    cout << "┌────────┬──────────────────────┬────────────────┐" << endl
         << "│        │     running time (ms)│         speedup│" << endl
         << "│    size├─────────┬──────┬─────┼─────────┬──────┤" << endl
         << "│        │GPU total│kernel│  CPU│GPU total│kernel│" << endl
         << "├────────┼─────────┼──────┼─────┼─────────┼──────┤" << endl;
    cout << setprecision(2) << fixed;
    for (auto &size : SCALES) {
        float *host_a     = new float[size];
        float *host_b     = new float[size];
        float *host_c_gpu = new float[size];
        float *host_c_cpu = new float[size];

        /* Generate random vector */
        generate_vector(host_a, host_b, size);

        /* Allocate device memory */
        float *device_a, *device_b, *device_c;
        cudaMalloc((void **)&device_a, size * sizeof(float));
        cudaMalloc((void **)&device_b, size * sizeof(float));
        cudaMalloc((void **)&device_c, size * sizeof(float));

        /* Define block and grid size */
        dim3 dim_block(NUM_THREADS, 1, 1);
        int  num_block = (size - 1) / NUM_THREADS + 1;
        dim3 dim_grid(num_block, 1, 1);

        /* Time the GPU additon */
        auto gpu_start = now();
        cudaMemcpy(device_a, host_a, size * sizeof(float),
                   cudaMemcpyHostToDevice);
        cudaMemcpy(device_b, host_b, size * sizeof(float),
                   cudaMemcpyHostToDevice);
        cudaDeviceSynchronize();

        auto kernel_start = now();
        vecadd_kernel<<<dim_grid, dim_block>>>(device_a, device_b, device_c,
                                               size);
        cudaDeviceSynchronize();
        auto kernel_end = now();

        cudaMemcpy(host_c_gpu, device_c, size * sizeof(float),
                   cudaMemcpyDeviceToHost);
        auto gpu_end = now();

        /* Time the CPU addition */
        auto cpu_start = now();
        vecadd_cpu(host_a, host_b, host_c_cpu, size);
        auto cpu_end = now();

        /* Verify the result */
        if (!verify(host_c_gpu, host_c_cpu, size)) {
            cout << "Verification failed on size " << size << endl;
            return 1;
        }

        /* Calculate the elapsed time */
        ns gpu_ns    = duration_cast<ns>(gpu_end - gpu_start);
        ns kernel_ns = duration_cast<ns>(kernel_end - kernel_start);
        ns cpu_ns    = duration_cast<ns>(cpu_end - cpu_start);

        float gpu_time    = gpu_ns.count() / 1e6;
        float kernel_time = kernel_ns.count() / 1e6;
        float cpu_time    = cpu_ns.count() / 1e6;

        float speedup        = cpu_time / gpu_time;
        float kernel_speedup = cpu_time / kernel_time;

        cout << "│" << setw(8) << size << "│" << setw(9) << gpu_time << "│"
             << setw(6) << kernel_time << "│" << setw(5) << cpu_time << "│"
             << setw(9) << speedup << "│" << setw(6) << kernel_speedup << "│"
             << endl;

        cudaFree(device_a);
        cudaFree(device_b);
        cudaFree(device_c);

        delete[] host_a;
        delete[] host_b;
        delete[] host_c_gpu;
        delete[] host_c_cpu;
    }
    cout << "└────────┴─────────┴──────┴─────┴─────────┴──────┘" << endl;
}
