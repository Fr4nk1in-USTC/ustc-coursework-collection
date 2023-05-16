#include <omp.h>
#include <stdio.h>

static long num_steps = 10000000;

double step;

#define NUM_THREADS 2

int main()
{
    int    i;
    double pi  = 0.0;
    double sum = 0.0;
    double x   = 0.0;
    step       = 1.0 / (double)num_steps;
    omp_set_num_threads(NUM_THREADS);  // 设置 2 线程
// 该子句表示 x, sum 变量对于每个线程是私有的
#pragma omp parallel private(i, x, sum)
    {
        int id;
        id = omp_get_thread_num();
        for (i = id, sum = 0.0; i < num_steps; i = i + NUM_THREADS) {
            x    = (i + 0.5) * step;
            sum += 4.0 / (1.0 + x * x);
        }
#pragma omp critical  // 指定代码段在同一时刻只能由一个线程进行执行
        pi += sum * step;
    }
    printf("%lf\n", pi);
}

// 共 2 个线程参加计算, 其中
//     线程 0 进行迭代步 0, 2, 4, ...
//     线程 1 进行迭代步 1, 3, 5, ....
// 当被指定为 critical 的代码段正在被 0 线程执行时, 1 线程的执行也到达该代码段,
// 则它将被阻塞直到 0 线程退出临界区
