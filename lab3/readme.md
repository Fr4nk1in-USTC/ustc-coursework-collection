# Part 1: malloc
## 代码
主要代码修改见 [`memlib.c`](alloc/malloclab/memlib.c) 和 [`mm.c`](alloc/malloclab/mm.c), 或者见 [commit `4a26fo8`](https://github.com/Fr4nk1in-USTC/USTC-2022-OS-Lab/commit/4a26f0879ce8e02df1e11468c9fc7a3fe8e816d4)

要进行 gdb 调试, 只需要在 [`trace/run.sh`](alloc/trace/run.sh) 中的倒数第二行加上 `-g` 编译选项, 然后将最后一行改成 `gdb ./workload`, 即
```shell
#! /bin/bash

TASKPATH=$PWD
MALLOCPATH=~/oslab/lab3/alloc/malloclab # 需要修改为你的libmem.so所在目录
export LD_LIBRARY_PATH=$MALLOCPATH:$LD_LIBRARY_PATH
cd $MALLOCPATH; make clean; make
cd $TASKPATH
g++ -g workload.cc -o workload -I$MALLOCPATH -L$MALLOCPATH -lmem -lpthread
echo 
gdb ./workload
```
最后的运行结果如下 (已省略 warning):
- `find_fit_first` 策略
```plaintext
before free: 0.971756; after free: 0.218191
time of loop 0 : 535ms
before free: 0.953879; after free:  0.21264
time of loop 1 : 608ms
before free:  0.94881; after free: 0.215474
time of loop 2 : 645ms
before free: 0.945629; after free: 0.213654
time of loop 3 : 589ms
before free: 0.942803; after free: 0.208883
time of loop 4 : 578ms
before free:  0.94426; after free: 0.211562
time of loop 5 : 582ms
before free: 0.941761; after free:  0.21026
time of loop 6 : 578ms
before free: 0.945016; after free: 0.211107
time of loop 7 : 626ms
before free: 0.939299; after free: 0.208857
time of loop 8 : 624ms
before free: 0.948793; after free: 0.210797
time of loop 9 : 609ms
before free: 0.946833; after free: 0.211653
time of loop 10 : 593ms
before free: 0.945299; after free: 0.214872
time of loop 11 : 584ms
before free: 0.948271; after free: 0.213376
time of loop 12 : 579ms
before free:  0.94821; after free: 0.210934
time of loop 13 : 592ms
before free: 0.947627; after free: 0.212048
time of loop 14 : 621ms
before free: 0.942984; after free: 0.211459
time of loop 15 : 575ms
before free: 0.939253; after free: 0.210235
time of loop 16 : 580ms
before free: 0.939312; after free:  0.21098
time of loop 17 : 618ms
before free: 0.939824; after free: 0.209901
time of loop 18 : 630ms
before free: 0.943164; after free: 0.211889
time of loop 19 : 629ms
```
- `find_fit_best` 策略
```plaintext
before free: 0.971756; after free: 0.218191
time of loop 0 : 523ms
before free: 0.972013; after free:  0.21671
time of loop 1 : 1522ms
before free:  0.97098; after free: 0.220482
time of loop 2 : 1626ms
before free: 0.967858; after free: 0.218559
time of loop 3 : 1578ms
before free: 0.964723; after free: 0.213772
time of loop 4 : 1588ms
before free: 0.966334; after free: 0.216446
time of loop 5 : 1511ms
before free:  0.96381; after free: 0.215139
time of loop 6 : 1515ms
before free: 0.967203; after free: 0.216064
time of loop 7 : 1541ms
before free:  0.96161; after free: 0.213785
time of loop 8 : 1508ms
before free: 0.971067; after free: 0.215661
time of loop 9 : 1484ms
before free: 0.968858; after free: 0.216605
time of loop 10 : 1508ms
before free: 0.967509; after free: 0.219928
time of loop 11 : 1527ms
before free: 0.970443; after free: 0.218291
time of loop 12 : 1484ms
before free: 0.970222; after free:   0.2158
time of loop 13 : 1563ms
before free:  0.97213; after free:  0.21738
time of loop 14 : 1516ms
before free: 0.967185; after free: 0.216827
time of loop 15 : 1527ms
before free: 0.963688; after free: 0.215693
time of loop 16 : 1542ms
before free: 0.963652; after free:  0.21637
time of loop 17 : 1508ms
before free: 0.964038; after free: 0.215213
time of loop 18 : 1541ms
before free: 0.967592; after free: 0.217349
time of loop 19 : 1583ms
```
简单计算可知, `find_fit_first` 的速度是 `find_fit_best` 的约 2.6 倍, 但后者的占用率提升不大, 所以在这个测试样例中, `find_fit_first` 的表现更好.