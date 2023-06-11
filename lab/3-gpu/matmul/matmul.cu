#include <chrono>
#include <cstddef>
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

const float MIN_FLOAT = 0;
const float MAX_FLOAT = 100;

const size_t BLOCK_SIZE     = 32;               // block of 16 * 16 threads
const size_t MATRIX_SIZES[] = {10, 100, 1000};  // matrix size

/**
 * Generate two random vectors `a` and `b` of size `size`.
 */
void generate_matrix(float *a, float *b, size_t size)
{
    random_device                    rd;
    mt19937                          gen(rd());
    uniform_real_distribution<float> dis(MIN_FLOAT, MAX_FLOAT);

    for (size_t i = 0; i < size * size; i++) {
        a[i] = dis(gen);
        b[i] = dis(gen);
    }
}

__global__ static void matmul_kernel(const float *a, const float *b, float *c,
                                     size_t size)
{
    __shared__ float a_sub[BLOCK_SIZE][BLOCK_SIZE];
    __shared__ float b_sub[BLOCK_SIZE][BLOCK_SIZE];

    /* Block indices */
    const size_t block_x = blockIdx.x;
    const size_t block_y = blockIdx.y;

    /* Thread indices */
    const size_t thread_x = threadIdx.x;
    const size_t thread_y = threadIdx.y;

    /* Number of blocks */
    const size_t num_sub = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

    float c_sub = 0;

    for (size_t sub = 0; sub < num_sub; sub++) {
        /* Load the sub-matrices into the shared memory */
        /* Each thread loads one element of each sub-matrix */
        size_t a_x = block_x * BLOCK_SIZE + thread_x;
        size_t a_y = sub * BLOCK_SIZE + thread_y;
        if (a_x < size and a_y < size) {
            a_sub[thread_x][thread_y] = a[a_x * size + a_y];
        } else {
            a_sub[thread_x][thread_y] = 0;
        }

        size_t b_x = sub * BLOCK_SIZE + thread_x;
        size_t b_y = block_y * BLOCK_SIZE + thread_y;
        if (b_x < size and b_y < size) {
            b_sub[thread_x][thread_y] = b[b_x * size + b_y];
        } else {
            b_sub[thread_x][thread_y] = 0;
        }

        __syncthreads();

        /* Multiply the two sub-matrices */
        for (size_t k = 0; k < BLOCK_SIZE; k++) {
            c_sub += a_sub[thread_x][k] * b_sub[k][thread_y];
        }

        __syncthreads();
    }

    size_t c_x = block_x * BLOCK_SIZE + thread_x;
    size_t c_y = block_y * BLOCK_SIZE + thread_y;
    if (c_x < size and c_y < size) {
        c[c_x * size + c_y] = c_sub;
    }
}

void matmul_cpu(const float *a, const float *b, float *c, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            float t = 0;
            for (size_t k = 0; k < size; k++) {
                t += a[i * size + k] * b[k * size + j];
            }
            c[i * size + j] = t;
        }
    }
}

bool verify(const float *a, const float *b, size_t size)
{
    for (size_t i = 0; i < size * size; i++) {
        if (fabs(a[i] - b[i]) / a[i] > 1e-6) {
            return false;
        }
    }
    return true;
}

void print_matrix(const float *a, size_t size)
{
    for (size_t i = 0; i < size * size; i++) {
        cout << setw(10) << fixed << setprecision(1) << a[i];
        if ((i + 1) % size == 0) {
            cout << endl;
        }
    }
}

int main()
{
    cout << "┌────┬────────────────────────┬────────────────┐" << endl
         << "│    │       running time (ms)│         speedup│" << endl
         << "│size├─────────┬──────┬───────┼─────────┬──────┤" << endl
         << "│    │GPU total│kernel│    CPU│GPU total│kernel│" << endl
         << "├────┼─────────┼──────┼───────┼─────────┼──────┤" << endl;
    cout << fixed << setprecision(2);
    for (auto &size : MATRIX_SIZES) {
        float *host_a     = new float[size * size];
        float *host_b     = new float[size * size];
        float *host_c_gpu = new float[size * size];
        float *host_c_cpu = new float[size * size];

        /* Generate random matrices. */
        generate_matrix(host_a, host_b, size);

        /* Allocate device memory. */
        float *device_a, *device_b, *device_c;
        cudaMalloc((void **)&device_a, size * size * sizeof(float));
        cudaMalloc((void **)&device_b, size * size * sizeof(float));
        cudaMalloc((void **)&device_c, size * size * sizeof(float));

        /* Define block and grid size */
        int grid_size = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;

        dim3 dim_block(BLOCK_SIZE, BLOCK_SIZE, 1);
        dim3 dim_grid(grid_size, grid_size, 1);

        /* Time the GPU multiplication */
        auto gpu_start = now();
        cudaMemcpy(device_a, host_a, size * size * sizeof(float),
                   cudaMemcpyHostToDevice);
        cudaMemcpy(device_b, host_b, size * size * sizeof(float),
                   cudaMemcpyHostToDevice);
        cudaDeviceSynchronize();

        auto kernel_start = now();
        matmul_kernel<<<dim_grid, dim_block, 0>>>(device_a, device_b, device_c,
                                                  size);
        cudaDeviceSynchronize();
        auto kernel_end = now();

        cudaMemcpy(host_c_gpu, device_c, size * size * sizeof(float),
                   cudaMemcpyDeviceToHost);
        auto cuda_end = now();

        /* Time the CPU multiplication */
        auto cpu_start = now();
        matmul_cpu(host_a, host_b, host_c_cpu, size);
        auto cpu_end = now();

        /* Verify the result */
        if (!verify(host_c_gpu, host_c_cpu, size)) {
            cout << "Verification failed on size " << size << endl;
            cout << "CPU result:" << endl;
            print_matrix(host_c_cpu, size);
            cout << "GPU result:" << endl;
            print_matrix(host_c_gpu, size);
            return 1;
        }

        /* Calculate the elapsed time */
        ns gpu_ns    = duration_cast<ns>(cuda_end - gpu_start);
        ns kernel_ns = duration_cast<ns>(kernel_end - kernel_start);
        ns cpu_ns    = duration_cast<ns>(cpu_end - cpu_start);

        float gpu_time    = gpu_ns.count() / 1e6;
        float kernel_time = kernel_ns.count() / 1e6;
        float cpu_time    = cpu_ns.count() / 1e6;

        float speedup        = cpu_time / gpu_time;
        float kernel_speedup = cpu_time / kernel_time;

        cout << "│" << setw(4) << size << "│" << setw(9) << gpu_time << "│"
             << setw(6) << kernel_time << "│" << setw(7) << cpu_time << "│"
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
    cout << "└────┴─────────┴──────┴───────┴─────────┴──────┘" << endl;
}
