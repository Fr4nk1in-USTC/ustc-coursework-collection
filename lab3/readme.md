# Tips
- Part 1 容易在 `coalesce()` 函数出问题, 大部分的 Segmentation Fault 都是由它引起的.
- Part 2 的 Func 2 容易出问题导致需要重启, 建议在运行它之前保存快照 (如果你是虚拟机的话), 重启强制关闭模块非常耗时.
- 建议仔细阅读 Part 2 试验文档的警告.

# Part 1: Alloc
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

# 2: Module
> 我的内核版本为 `5.13.0-40-generic`, 具体代码实现可能有所不同.
## Func 1
Func 1 就是 `vma` 链表遍历并计数, 链表头为 `mm->mmap`, 尾为 `NULL`, 代码如下:
```c
static void scan_vma(void)
{
    printk("func == 1, %s\n", __func__);
    struct mm_struct *mm = get_task_mm(my_task_info.task);
    if (mm)
    {
        struct vm_area_struct *vma = mm->mmap;

        my_task_info.vma_cnt = 0;
        while (!IS_ERR_OR_NULL(vma))
        {
            vma = vma->vm_next;
            my_task_info.vma_cnt++;
        }
        mmput(mm);
    }
    else
    {
        pr_err("func: %s mm_struct is NULL\n", __func__);
    }
}
```
使用脚本 `./run_expr.sh 2` 运行该功能, 要获取运行结果, 只需要
```shell
cat /sys/kernel/mm/ktest/vma
```
- 在我的 Ubuntu 20.04 虚拟机上运行结果为
    ```plaintext
    0, 39
    ```
- 在我的 Ubuntu 21.10 实体机 (双系统) 上运行结果为
    ```plaintext
    0, 40
    ```

## Func 2
Func 2 遍历 `vma` 链表的每一个 `vma` 的每一个虚拟地址, 并获取其访问频率, 如果频率大于 0 就调用 `record_one_data()` 接口记录 page 的物理地址. 代码如下
```c
static void print_mm_active_info(void)
{
    printk("func == 2, %s\n", __func__);
    struct mm_struct      *mm = get_task_mm(my_task_info.task);
    struct vm_area_struct *vma;
    struct page           *page;
    unsigned long virt_addr;
    unsigned long vm_flags;
    int freq;
    if (mm)
    {
        vma = mm->mmap;
        while (!IS_ERR_OR_NULL(vma))
        {
            for (virt_addr = vma->vm_start; virt_addr < vma->vm_end; virt_addr += PAGE_SIZE)
            {
                page = mfollow_page(vma, virt_addr, FOLL_GET);
                if (IS_ERR_OR_NULL(page)) continue;
                freq = mpage_referenced(page, 0, (struct mem_cgroup *)(page->memcg_data), &vm_flags);
                if (freq > 0) record_one_data(page_to_pfn(page));
            }
            vma = vma->vm_next;
        }
        flush_buf(1);
        mmput(mm);
    }
    else
    {
        pr_err("func: %s mm_struct is NULL\n", __func__);
    }
}
```
使用脚本 `./run_expr.sh 2` 运行该功能, 它会画出页面冷热图, 如下
![](/lab3/module/page_fre_pic.jpg)

如果无法画图, 检查系统内是否有 Python 3 以及 matplotlib 库, 可以使用如下指令安装它们
```shell
sudo apt install python3
sudo apt install python3-pip
sudo python3 -m pip install matplotlib
```


## Func 3
### 查找多级页表
使用 `p**_offset` 宏/函数一级一级找到页面, 然后返回其物理地址, 注意判断各个返回值是否有效, 代码如下
```c
static unsigned long virt2phys(struct mm_struct *mm, unsigned long virt)
{
    struct page *page = NULL;
    pgd_t *pgd = pgd_offset(mm, virt);
    if (pgd_none(*pgd) || pgd_bad(*pgd))
    {
        pr_err("func: %s pgd is not valid\n", __func__);
        return NULL;
    }
    pud_t *pud = pud_offset((p4d_t *)pgd, virt);
    if (pud_none(*pud) || pud_bad(*pud))
    {
        pr_err("func: %s pud is not valid\n", __func__);
        return NULL;
    }
    pmd_t *pmd = pmd_offset(pud, virt);
    if (pmd_none(*pmd) || pmd_bad(*pmd))
    {
        pr_err("func: %s pmd is not valid\n", __func__);
        return NULL;
    }
    pte_t *pte = pte_offset_kernel(pmd, virt);
    if (!pte)
    {
        pr_err("func: %s pte is not valid\n", __func__);
        return NULL;
    }
    page = pte_page(*pte);

    if (page)
    {
        return page_to_pfn(page);
    }
    else
    {
        pr_err("func: %s page is NULL\n", __func__);
        return NULL;
    }
}
```
### 打印所有页面物理号
与上面 Func 2 类似, 这次使用 `record_two_data()` 接口. 代码如下
```c
static void traverse_page_table(void)
{
    printk("func == 3, %s\n", __func__);
    struct mm_struct      *mm = get_task_mm(my_task_info.task);
    struct vm_area_struct *vma;
    unsigned long virt_addr;
    if (mm)
    {
        vma = mm->mmap;
        while (!IS_ERR_OR_NULL(vma))
        {
            for (virt_addr = vma->vm_start; virt_addr < vma->vm_end; virt_addr += PAGE_SIZE)
            {
                record_two_data(virt_addr, virt2phys(mm, virt_addr));
            }
            vma = vma->vm_next;
        }
        flush_buf(1);
        mmput(mm);
    }
    else
    {
        pr_err("func: %s mm_struct is NULL\n", __func__);
    }
}
```
使用脚本 `./run_expr.sh 3` 运行该功能, 部分运行结果如下:
```plaintext
0x56376a3ea000--0x138e00
0x56376a3eb000--0x2e5df9
0x56376a3ec000--0x321650
0x56376a3ed000--0x0
0x56376a3ee000--0x35bb12
```
## Func 4 & 5
- 首先由 `mm->start_data` 和 `mm->end_data` (`mm->start_code` 和 `mm->end_dode`) 获取数据段 (代码段) 的起止虚拟地址, 注意到它们不一定是 `PAGE_SIZE` 的整数倍, 所以要在它们所在的所有页面内遍历. 
- 使用 `find_vma()` 函数找到虚拟地址对应的 `vma`, 再用 `mfollow_page()` 找到对应的页面. 
- 用 `kmap_atomic()` 和 `kunmap_atomic()` 得到页面可直接访问的虚拟地址.
- 然后计算出相应的地址写入 `buf`, 再写入文件中.

代码如下
```c
static void print_seg_info(void)
{
    struct mm_struct      *mm;
    struct vm_area_struct *vma;
    struct page           *page;
    unsigned long seg_start;
    unsigned long seg_end;
    unsigned long start_addr;
    unsigned long end_addr;
    unsigned long virt_addr;
    unsigned long page_addr;
    unsigned long buf_start;
    unsigned long buf_size;
    printk("func == 4 or func == 5, %s\n", __func__);
    mm = get_task_mm(my_task_info.task);
    if (mm == NULL)
    {
        pr_err("func: %s mm_struct is NULL\n", __func__);
        return;
    }
    if (ktest_func == 4)
    {
        seg_start = mm->start_data;
        seg_end   = mm->end_data;
    }
    else
    {
        seg_start = mm->start_code;
        seg_end   = mm->end_code;
    }
    start_addr = (seg_start >> PAGE_SHIFT) << PAGE_SHIFT;
    end_addr   = (seg_end >> PAGE_SHIFT) << PAGE_SHIFT;
    for (virt_addr = start_addr; virt_addr <= end_addr; virt_addr += PAGE_SIZE)
    {
        vma = find_vma(mm, virt_addr);
        if (IS_ERR_OR_NULL(vma)) continue;
        page = mfollow_page(vma, virt_addr, FOLL_GET);
        if (IS_ERR_OR_NULL(page)) continue;
        page_addr = kmap_atomic(page);
        kunmap_atomic(page_addr);
        buf_start = page_addr + my_max(virt_addr, seg_start) - virt_addr;
        buf_size  = my_min(virt_addr + PAGE_SIZE, seg_end) - my_max(virt_addr, seg_start);
        memcpy(buf, buf_start, buf_size);
        curr_buf_length = my_min(PAGE_SIZE, buf_size);
        flush_buf(0);
    }
    mmput(mm);
}
```
使用脚本 `./run_expr.sh 4` (`./run_expr.sh 5`) 运行该功能, 在输出文件中能看到 dump 出的内容.