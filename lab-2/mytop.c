#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/syscall.h>

#define TASK_COMM_LEN 16
#define MAX_TASK_NUM  32767
typedef struct info
{
    char comm[TASK_COMM_LEN];
    pid_t pid;
    long state;
    unsigned long long runtime;
} info_t;

/**
 * @brief 对进程信息 qsort 的比较函数 (降序)
 */
int cmp(const void * a, const void * b)
{
    return (((info_t *)b)->runtime - ((info_t *)a)->runtime);
}

int main(void)
{
    int ps_num;      /* 当前进程数 */
    int ps_num_last; /* 上一秒进程数 */
    info_t * curr  = (info_t *)malloc(MAX_TASK_NUM * sizeof(info_t)); /* 当前进程信息 */
    info_t * bakup = (info_t *)malloc(MAX_TASK_NUM * sizeof(info_t)); /* 当前进程信息的拷贝 */
    info_t * last  = (info_t *)malloc(MAX_TASK_NUM * sizeof(info_t)); /* 上一秒的进程信息 */
    
    char input = 0;
    /* 获取初始状态下各进程的信息 */
    system("clear");
    syscall(333, last, &ps_num_last);
    sleep(1);
    while (1) {
        system("clear");
        syscall(333, curr, &ps_num);
        memcpy(bakup, curr, ps_num * sizeof(info_t));
        /* 计算上一秒中进程的运行时间, 如果进程是新出现的, 则运行时间记为 0 */
        for (int i = 0; i < ps_num; i++) {
            int flag = 0;
            for (int j = 0; j < ps_num_last; j++) {
                if (curr[i].pid == last[j].pid) {
                    curr[i].runtime -= last[j].runtime;
                    flag = 1;
                    break;
                }
            }
            if (!flag)
                curr[i].runtime = 0;
        }
        /* 保存当前进程信息 */
        memcpy(last, bakup, ps_num * sizeof(info_t));
        ps_num_last = ps_num;
        /* 对 CPU 占用进行排序 */
        qsort(curr, ps_num, sizeof(info_t), cmp);
        /* 打印进程信息 */
        printf("PID             COMM    CPU ISRUNNING\n");
        for (int i = 0; i < 20 && i < ps_num; i++)
        {
            printf("%3d %16s %5.2f%%         %d\n",
                curr[i].pid,
                curr[i].comm,
                curr[i].runtime / (float)10000000.0,
                curr[i].state == 0
            );
        }
        sleep(1);
    }
    free(curr);
    free(bakup);
    free(last);
    return 0;
}