#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_CMDLINE_LENGTH 1024 /* 最大单行命令行长度 */
#define MAX_BUF_SIZE       4096 /* 最大 buffer 大小 */
#define MAX_CMD_ARG_NUM    32   /* 最大单命令参数数 */

#define WRITE_END 1 /* 管道写端 */
#define READ_END 0  /* 管道读端 */

/**
 * @brief  基于分隔符 sep 对于 string 做分割,
 *         并去掉头尾的空格
 *
 * @param  string:       被分割的字符串
 * @param  sep:          分隔符
 * @param  string_clips: 分割后的字符串数组
 *
 * @retval int, 分割的段数
 */
int split_string(char *string, char *sep, char **string_clips)
{
    char string_dup[MAX_BUF_SIZE];
    string_clips[0] = strtok(string, sep);
    int clip_num = 0;

    do {
        char *head, *tail;
        head = string_clips[clip_num];
        tail = head + strlen(string_clips[clip_num]) - 1;

        while (*head == ' ' && head != tail)
            head++;
        while (*tail == ' ' && tail != head)
            tail--;
        
        *(tail + 1) = '\0';
        string_clips[clip_num] = head;
        clip_num++;
    } while (string_clips[clip_num] = strtok(NULL, sep));

    return clip_num;
}

/**
 * @brief  执行内置命令
 * @note   第一个参数就是要执行的命令, 若执行"ls a b c"命令, 
 *         则argc=4, argv={"ls", "a", "b", "c"}
 * 
 * @param  argc: 命令的参数个数
 * @param  argv: 依次代表每个参数
 * @param  fd:   命令输入和输出的文件描述符
 * 
 * @return int
 * @retval 0 - 执行成功
 * @retval -1 - 不是内置指令
 * @retval -2 - 内置命令执行失败
 */
int exec_builtin(int argc, char **argv, int *fd)
{
    if (argc == 0)
        return 0;
    
    /* DONE: 添加和实现内置指令 */
    if (strcmp(argv[0], "cd") == 0) {
        if (argc > 2) {
            printf("cd: too many arguments\n");
            return -2;
        }

        char *path;
        if (argc == 1)
            path = getenv("HOME");
        else 
            path = argv[1];

        if (chdir(path) == -1) {
            printf("cd: no such file or directory: %s\n", path);
            return -2;
        }
        return 0;
    } else if (strcmp(argv[0], "exit") == 0){
        if (argc > 2) {
            printf("exit: too many arguments\n");
            return -2;
        }

        int exit_code = 0;
        if (argc == 2)
            exit_code = atoi(argv[1]);
        
        exit(exit_code);
    } else {
        // 不是内置指令
        return -1;
    }
}

/**
 * @brief  实现重定向符, 从 argv 中删除重定向符和随后的参数, 
 *         并打开对应的文件, 将文件描述符放在fd数组中.
 * 
 * @note   第一个参数就是要执行的命令, 若执行"ls a b c"命令, 
 *         则argc=4, argv={"ls", "a", "b", "c"}
 * 
 * @param  argc: 命令的参数个数
 * @param  argv: 依次代表每个参数
 * @param  fd:   命令输入和输出使用的文件描述符
 * 
 * @retval int, 返回处理过重定向后命令的参数个数
 */
int process_redirect(int argc, char **argv, int *fd)
{
    /* 默认输入输出到命令行 */
    fd[READ_END]  = STDIN_FILENO;
    fd[WRITE_END] = STDOUT_FILENO;

    int i = 0, j = 0;
    while(i < argc) {
        int tfd;
        if(strcmp(argv[i], ">") == 0) {
            tfd = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0666);

            if(tfd < 0)
                printf("open '%s' error: %s\n", argv[i+1], strerror(errno));
            else
                fd[WRITE_END] = tfd;
            
            i += 2;
        } else if(strcmp(argv[i], ">>") == 0) {
            tfd = open(argv[i+1], O_WRONLY | O_CREAT | O_APPEND, 0666);

            if(tfd < 0)
                printf("open '%s' error: %s\n", argv[i+1], strerror(errno));
            else
                fd[WRITE_END] = tfd;

            i += 2;
        } else if(strcmp(argv[i], "<") == 0) {
            tfd = open(argv[i+1], O_RDONLY, 0666);

            if(tfd < 0)
                printf("open '%s' error: %s\n", argv[i+1], strerror(errno));
            else
                fd[READ_END] = tfd;
            
            i += 2;
        } else {
            argv[j++] = argv[i++];
        }
    }
    argv[j] = NULL;
    return j;
}

/**
 * @brief  在进程中执行命令并在执行完毕后结束进程
 * @note   第一个参数就是要执行的命令, 若执行"ls a b c"命令, 
 *         则argc=4, argv={"ls", "a", "b", "c"}
 * 
 * @param  argc: 命令的参数个数
 * @param  argv: 依次代表每个参数
 * 
 * @retval int, 若执行成功则不会返回 (进程直接结束), 否则返回非零
 */
int execute(int argc, char **argv) {
    int fd[2];
    // 默认输入输出到命令行，即输入STDIN_FILENO，输出STDOUT_FILENO 
    fd[READ_END]  = STDIN_FILENO;
    fd[WRITE_END] = STDOUT_FILENO;
    // 处理重定向符，如果不做本部分内容，请注释掉process_redirect的调用
    argc = process_redirect(argc, argv, fd);
    if(exec_builtin(argc, argv, fd) == 0) {
        exit(0);
    }
    // 将标准输入输出STDIN_FILENO和STDOUT_FILENO修改为fd对应的文件
    dup2(fd[READ_END],  STDIN_FILENO);
    dup2(fd[WRITE_END], STDOUT_FILENO);
    // 运行命令与结束 
    int ret = execvp(argv[0], argv);
    if (ret < 0)
        printf("%s: %s\n", argv[0], strerror(errno));
    return -1;
}

/**
 * @brief  执行单个命令
 * @note   
 * @param  command: 命令行
 * @retval None
 */
void execute_command(char *command)
{
    char *commands[128];
    int  cmd_count;
    cmd_count = split_string(command, "|", commands);

    if (cmd_count == 0) {
        return ;
    } else if (cmd_count == 1) {
        char *argv[MAX_CMD_ARG_NUM];
        int  argc;
        int  fd[2];
        // 处理参数，分出命令名和参数
        argc = split_string(commands[0], " ", argv);
        // 内建命令在主进程中完成
        if (exec_builtin(argc, argv, NULL) != -1)
            return ;
        // 外部命令在子进程中完成
        pid_t pid = fork();
        if (pid == 0) {
            if (execute(argc, argv) != 0)
                exit(255);
        }
        while (wait(NULL) > 0);
        return ;
    } else if (cmd_count == 2) {  // 两个命令间的管道
        int pipefd[2];
        int ret = pipe(pipefd);
        if (ret < 0) {
            printf("pipe error!\n");
            return ;
        }
        // 子进程 1
        pid_t pid = fork();
        if (pid == 0) {
            // 将标准输出重定向到管道
            close(pipefd[READ_END]);
            dup2(pipefd[WRITE_END], STDOUT_FILENO);
            close(pipefd[WRITE_END]);
            // 执行指令
            char *argv[MAX_CMD_ARG_NUM];
            int  argc;
            argc = split_string(commands[0], " ", argv);
            if (exec_builtin(argc, argv, NULL) != -1)
                exit(0);
            execute(argc, argv);
            exit(255);
        }
        // 子进程 2
        pid = fork();
        if (pid == 0) {
            // 将标准输入重定向到管道
            close(pipefd[WRITE_END]);
            dup2(pipefd[READ_END], STDIN_FILENO);
            close(pipefd[READ_END]);
            // 执行指令
            char *argv[MAX_CMD_ARG_NUM];
            int  argc;
            argc = split_string(commands[1], " ", argv);
            if (exec_builtin(argc, argv, NULL) != -1)
                exit(0);
            execute(argc, argv);
            exit(255);
        }
        // 父进程
        close(pipefd[WRITE_END]);
        close(pipefd[READ_END]);
        while (wait(NULL) > 0);
        return ;
    } else { // 选做: 三个以上的命令
        int read_fd; // 上一个管道的读端口

        for (int i = 0; i < cmd_count; i++)
        {
            int pipefd[2];
            if (i != cmd_count - 1) {
                int ret = pipe(pipefd);
                if (ret < 0) {
                    printf("pipe error!");
                    return ;
                }
            }
            pid_t pid = fork();
            if (pid == 0) {
                // 除了最后一条命令外, 都将标准输出重定向到当前管道入口
                if (i != cmd_count - 1) {
                    close(pipefd[READ_END]);
                    dup2(pipefd[WRITE_END], STDOUT_FILENO);
                    close(pipefd[WRITE_END]);
                }
                // 除了第一条命令外, 都将标准输入重定向到上一个管道出口
                if (i != 0) {
                    dup2(read_fd, STDIN_FILENO);
                    close(read_fd);
                }
                // 执行指令
                char *argv[MAX_CMD_ARG_NUM];
                int  argc;
                argc = split_string(commands[i], " ", argv);
                if (exec_builtin(argc, argv, NULL) != -1)
                    exit(0);
                execute(argc, argv);
                exit(255);
            }
            close(pipefd[WRITE_END]);
            if (i != 0)
                close(read_fd);
            if (i != cmd_count - 1) 
                read_fd = pipefd[READ_END];
        }
        for (int i = 0; i < cmd_count; i++) {
            while (wait(NULL) < 0) ;
        }
        return ;
    }
}

int main()
{
    /* 输入的命令行 */
    char cmdline[MAX_CMDLINE_LENGTH];

    char *commands[128];
    int  cmd_count;

    while (1) {
        // 打印当前目录
        char *pwd = getcwd(NULL, 0);
        printf("shell:%s -> ", pwd);
        free(pwd);
        fflush(stdout);

        // 获取命令行
        fgets(cmdline, MAX_CMDLINE_LENGTH, stdin);
        strtok(cmdline, "\n");

        // 基于 ";" 的多命令执行
        cmd_count = split_string(cmdline, ";", commands);
        for (int i = 0; i < cmd_count; i++) {
            execute_command(commands[i]);
        }
    }

}