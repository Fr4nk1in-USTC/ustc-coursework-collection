# 知识问答
1.  解释 `wc` 和 `grep` 指令的含义.
    - `wc` 统计字数, `-l` 参数表示统计行数, `-w` 参数表示统计词数.
    - `grep` 筛选并高亮指定字符.
2.  解释 `ps aux | grep firefox | wc -l` 的含义.
    - 显示所有进程名含 `firefox` 的数量.
3.  `echo aaa | echo bbb | echo ccc` 是否适合做 shell 实验中管道符的检查用例? 说明原因.
    - 不适合, 因为 `echo` 命令本身不接受输入.
4.  对于匿名管道, 如果写端不关闭, 并且不写, 读端会怎样?
    - 读端会先读完管道中内容, 读完后继续等待, 此时阻塞, 直到有数据写入才继续. 
5.  对于匿名管道, 如果读端关闭, 但写端仍尝试写入, 写端会怎样?
    - 写进程收到信号 `SIGPIPE`, 通常导致进程异常中止.
6.  假如使用匿名管道从父进程向子进程传输数据, 此时子进程不写数据, 为什么子进程要关闭管道的写端?
    - 如果不关闭子进程的写端, 子进程会一直等待.
7.  fork 之后, 是管道从一分为二, 变成两根管道了吗? 如果不是, 复制的是什么?
    - 不是管道一分为二.
    - 复制的是管道读写两端的文件描述符
8.  解释系统调用 `dup2` 的作用.
    - `int dup2(int oldfd, int newfd);`
    - 将 `newfd` 标识符变成 `oldfd` 的一个拷贝, 与 `newfd` 相关的输入输出都会重定向到 `oldfd` 中.
    - 如果 `newfd` 之前已被打开, 则先将其关闭.
9.  什么是 shell 内置指令, 为什么不能 `fork` 一个子进程然后 `exec cd`?
    - 是直接在 shell 进程中运行的命令, 它可以改变当前 shell 的参数.
    - 如果在子进程中运行 `exec cd`, 子进程运行结束后会回到父进程的环境, shell 的路径根本没有被改变.
10. 为什么 `ps aux | wc -l` 得出的结果比 `get_ps_num` 多 2?
    - `ps aux` 第一行是各状态名, 而且 `ps` 进程和 `wc` 进程也会被统计, 所以 `ps aux | wc -l` 的输出实际上是运行前进程数 +3.
    - `get_ps_num` 进程会被自身统计, 输出为运行前进程数 +1.
    - 所以 `ps aux | wc -l` 的结果比 `get_ps_num` 多 2.
11. 进程名的最大长度是多少? 这个长度在哪定义?
    - 16 (数组大小, 实际上最长为 15)
    - 在 [`/include/linux/sched.h:316`](https://elixir.bootlin.com/linux/v4.9.263/source/include/linux/sched.h#L316) 中定义: `#define TASK_COMM_LEN 16`
      - 进程名在 [`/include/linux/sched.h:1680`](https://elixir.bootlin.com/linux/v4.9.263/source/include/linux/sched.h#L1680) 定义 `char comm[TASK_COMM_LEN];`
12. `task_struct` 在 Linux 源码的哪个文件中定义?
    - 在 [`/include/linux/sched.h:1487`](https://elixir.bootlin.com/linux/v4.9.263/source/include/linux/sched.h#L1487) 中定义.
13. *为什么无法通过 `SYSCALL_DEFINEx` 定义二维数组 (如 `char (*p)[50]`) 为参数?
    - `SYSCALL_DEFINEx` 会将类型名与变量名进行简单地拼接
    - 在类型 `char (*p)[50]` 中, 类型名与变量名无法分割.
      - 如果分割为 `char (*)[50]`, `p` 则定义时会变成 `char (*)[50] p`, 不符合 C 语言语法.
14. 在修改内核代码的时候, 能用 `printf` 调试吗? 如果不能, 应该用什么调试?
    - 内核中无法使用标准库函数, 所以不能用 `printf()`
    - 应该用 `printk()`
15. `read()`, `write()`, `dup2()` 都能直接调用. 现在我们已经写好了一个名为 `ps_counter` 的系统调用. 为什么我们不能在测试代码中直接调 `ps_counter()` 来调用系统调用?
    - Linux 使用的开源标准 C 运行库 GNU libc 中的头文件 `unistd.h` 中声明了很多封装好的系统调用, 其中包括了 `read()`, `write()`, `dup2()`, 所以它们能直接调用.
    - 我们自己写的系统调用并没有被封装.

# Shell 部分
见[shell.c](./shell.c)文件, 实现了:
- [x] 支持基本的单条命令运行
- [x] 支持两条命令间的管道 `|`
- [x] 支持内建命令 `cd` / `exit`
- [x] 选做
  - [x] 支持多条命令间的管道 `|` 操作
  - [x] 支持重定向符 `>`, `>>`, `<` 和分号 `;`

编译选项:
```shell
gcc -O2 shell.c -o shell
```
运行实例:
```plaintext
shell:/home/xxx -> cd test
shell:/home/xxx/test -> ps aux | wc -l
326
shell:/home/xxx/test -> ps aux | grep code | wc -l
20
shell:/home/xxx/test -> echo 12345 > a
shell:/home/xxx/test -> cat a
12345
shell:/home/xxx/test -> echo 23456 >> a 
shell:/home/xxx/test -> cat a
12345
23456
shell:/home/xxx/test -> grep 6 < a         
23456
shell:/home/xxx/test -> mkdir test 
shell:/home/xxx/test -> cd test ; touch example ; ls ; ps aux | wc -l ; echo abcde > example ; cat example
example
325
abcde
shell:/home/xxx/test/test -> exit
```

# 编写系统调用实现 `top`
系统调用名为 `ps_info`
- 注册表见 [syscall_64.tbl](../linux-4.9.263/arch/x86/entry/syscalls/syscall_64.tbl) 的第 387 行.
- 内核函数原型见 [syscalls.h](../linux-4.9.263/include/linux/syscalls.h) 的第 908 行.
- 内核函数见 [sys.c](../linux-4.9.263/kernel/sys.c) 的第 2471 行.

修改完后, 在 linux 源码路径下执行
```shell
make
```

自己编写的 `top` 见 [mytop.c](mytop.c). 实现了
- [x] 在**用户态**每秒打印一次各进程的CPU占用统计,并按CPU占用统计排序
- [x] 每秒打印一次各进程的 PID, 进程名, 进程是否处于 running 状态

在命令行执行
```shell
gcc -static mytop.c -o mytop
sudo cp mytop ../busybox-1.32.1/_install
cd ../busybox-1.32.1/_install
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../../initramfs-busybox-x64.cpio.gz
cd ../..
./qemu.sh # 也可以输入指令运行 qemu
```

在 qemu 中运行如下
```plaintext
[   22.991868] [Syscall] ps_info
PID             COMM    CPU ISRUNNING
965            mytop  0.66%         1
853      kworker/0:2  0.11%         0
  7        rcu_sched  0.05%         0
  1               sh  0.00%         0
  2         kthreadd  0.00%         0
  3      ksoftirqd/0  0.00%         0
  4      kworker/0:0  0.00%         0
  5     kworker/0:0H  0.00%         0
  6     kworker/u2:0  0.00%         0
  8           rcu_bh  0.00%         0
  9      migration/0  0.00%         0
 10    lru-add-drain  0.00%         0
 11          cpuhp/0  0.00%         0
 12        kdevtmpfs  0.00%         0
 13            netns  0.00%         0
 14     kworker/u2:1  0.00%         0
154     kworker/u2:2  0.00%         0
335     kworker/u2:3  0.00%         0
404       oom_reaper  0.00%         0
405        writeback  0.00%         0
```

## 现场阅读 Linux 源码展示 `pid` 数据类型
`pid` 在[源码](https://elixir.bootlin.com/linux/v4.9.263/source)的 [`/include/linux/sched.h:1608`](https://elixir.bootlin.com/linux/v4.9.263/source/include/linux/sched.h#L1608) 定义. 

```c
	pid_t pid;
```

多个 `typedef` 如下

在 [`/include/linux/types.h:21`](https://elixir.bootlin.com/linux/v4.9.263/source/include/linux/types.h#L21) 中
```c
typedef __kernel_pid_t		pid_t;
```
在 [`/include/uapi/asm-generic/posix_types.h:27`](https://elixir.bootlin.com/linux/v4.9.263/source/include/uapi/asm-generic/posix_types.h#L27) 中
```c
#ifndef __kernel_pid_t
typedef int		__kernel_pid_t;
#endif
```
所以 `pid` 的默认数据类型是 `int`