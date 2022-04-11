#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/syscall.h>

#define TASK_COMM_LEN 16
#define STATE_STR_LEN 5
#define MAX_TASK_NUM  32767
typedef struct info
{
    char comm[TASK_COMM_LEN];
    pid_t pid;
    long state;
    unsigned long long runtime;
} info_t;


int cmp(const void * a, const void * b)
{
    return (((info_t *)b)->runtime - ((info_t *)a)->runtime);
}


int main(void)
{
    int ps_num;
    int ps_num_last;
    info_t * curr  = (info_t *)malloc(MAX_TASK_NUM * sizeof(info_t));
    info_t * bakup = (info_t *)malloc(MAX_TASK_NUM * sizeof(info_t));
    info_t * last  = (info_t *)malloc(MAX_TASK_NUM * sizeof(info_t));
    syscall(333, curr, &ps_num);
    memcpy(last, curr, ps_num * sizeof(info_t));
    ps_num_last = ps_num;
    sleep(1);
    while (1) {
        syscall(333, curr, &ps_num);
        memcpy(bakup, curr, ps_num * sizeof(info_t));
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
        memcpy(last, bakup, ps_num * sizeof(info_t));
        ps_num_last = ps_num;
        qsort(curr, ps_num, sizeof(info_t), cmp);
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
        system("clear");
    }
}