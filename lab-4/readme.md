> 代码使用 clang-format 格式化, ColumnLimit 为 85, 你可能需要使用宽屏获得较好的代码阅读体验
# Tips
- 在 Part 1 开始之前可以看看 `find_root()` 和 `find_subdir()` 函数, 其遍历方法就是需要填入的代码的遍历方法, `find_root()` 函数也是之后需要调用的函数.
- 目录项 `0x00` 偏移 (即 `DIR_Name[0]` 位置) 为 `0x00` 时表示空闲项, `0xe5` 表示已删除项, `0x0f` 表示长文件名项.
- `find_root()` 返回 0 时表示找到
- `DIR_ENTRY` 结构体的 `DIR_Name` 元素并不是字符串, 因为其没有以 `\0` 结尾, 所以与 `paths[-1]` 比较时应该使用 `strncmp()` 函数, 返回 0 是表示两字符串相等.

# 知识问答
1.  FAT16 文件的目录名等元数据存储在哪? 数据存储在哪, 是怎么组织的?
    - 文件的元数据信息以目录项的形式存储在根目录 (当文件处于根目录下) 或子目录的数据区域 (当文件处于子目录下). 
    - 文件的数据存储在数据区域, 按簇以链表的形式组织, 下一个簇的地址存储在 FAT 表中.
2.  FAT16 系统中需要两个 FAT 表的目的是什么?
    - 出于系统冗余考虑
3.  FAT16 中的根目录和子目录在磁盘存储上有什么区别?
    - 根目录存储在专门的根目录区, 在 FAT16 中它永远紧随 FAT 区域之后, 大小一定是整数个扇区大小, 没有自己的目录项
    - 子目录作为文件存储在数据区域, 有自己的目录项, 大小取决于其中的目录项数.
4.  FAT16 的文件名存储格式是怎样的, 如何转化成实际用户看到的文件名?
    - 非 LFN: 8 位文件名 + 3 位拓展名 (过长则截断, 不足则空格补全), 全部用大写存储. 转化方式: 大写转小写, 去除空格并在文件名和扩展名之间加 `.`
    - LFN: 使用一个或多个目录项属性为 `0x0F` 的 LFN 项存储文件名.
5.  不考虑 LFN, FAT16 文件系统中支持的最大文件名长度是多少, 为什么?
    - 最大文件名长度为 8 字节: `DIR_Name` 的 11 字节减去拓展名的 3 字节.
6.  FAT16 文件系统的最大单文件的大小是多少, 受限于什么因素?
    - FAT16 中一个簇号为 16 位, 因此最多有 2^16 个簇, 且簇号 `0x0000`, `0x0001`, `0xFFF0` - `0xFFFF` 无法被使用, 而簇的大小一般不超过 32K = 2^15 字节, 所以最大单文件大小为 (2^16-18)*2^15 字节, 约 2GBytes.
    - 如果考虑文件目录项的大小段, 其长度为 4 字节, 所能支持的最大文件大小为 2^32 = 4GBytes.
7.  FAT16 文件系统分配空间的基本单位是什么, 文件系统读写磁盘的基本单位是什么?
    - 分配空间的基本单位为簇
    - 读写磁盘的基本单位为扇区
8.  FAT16 文件系统的根目录数量达到最大值的时候, 再次创建目录时, 会自动分配空闲簇吗?
    - 不会自动分配空闲簇, 因为根目录数量最大值是固定的.
9.  FAT16 文件系统某个文件夹下的所有目录项都是紧密排列的吗?
    - 根目录下所有目录项都是紧密排列的, 其中可能包含被删除的目录项和 LFN 项, 所以可用的目录项不是紧密排列的.
    - 子目录可能跨簇, 因为子目录分配的簇不一定是连续的, 所以其下的目录项不一定是紧密排列的.
10. FAT16 文件系统的布局是什么样的? 分为哪几个部分, 它们的位置在哪?
    - 依次为
      - 保留扇区
        - 引导扇区
        - 文件系统信息扇区
        - 保留空间
      - FAT 表区
      - 根目录区
      - 数据区

# Part 1 (Completed in 6 Hour)

## Task 1: FAT16 文件系统读操作

### 读目录 `fat16_readdir()`
参考 `find_root()` 和 `find_subdir()`, 使用 `path_decode()` 解析文件名, 使用 `filler(buffer, 文件名, NULL, 0)` 填充 `buffer`.
```c
int fat16_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi)
{
    FAT16 *fat16_ins = get_fat16_ins();

    BYTE sector_buffer[BYTES_PER_SECTOR];
    int  RootDirCnt = 1, DirSecCnt = 1;

    // 如果要读取的目录是根目录
    if (strcmp(path, "/") == 0) {
        DIR_ENTRY Root;
        sector_read(fat16_ins->fd, fat16_ins->FirstRootDirSecNum, sector_buffer);
        for (uint i = 1; i <= fat16_ins->Bpb.BPB_RootEntCnt; i++) {
            /*** BEGIN ***/
            memcpy(&Root,
                   sector_buffer + ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR,
                   BYTES_PER_DIR);

            int is_empty   = (Root.DIR_Name[0] == 0x00);
            int is_deleted = (Root.DIR_Name[0] == 0xE5);
            int is_lfn     = (Root.DIR_Attr == 0x0F);
            if (is_empty)
                break;
            if (!is_deleted && !is_lfn) {
                char *name = (char *)path_decode(Root.DIR_Name);
                filler(buffer, name, NULL, 0);
                free(name);
            }
            /*** END ***/
            // 当前扇区所有条目已经读取完毕, 将下一个扇区读入 sector_buffer
            if (i % 16 == 0 && i != fat16_ins->Bpb.BPB_RootEntCnt) {
                sector_read(fat16_ins->fd,
                            fat16_ins->FirstRootDirSecNum + RootDirCnt,
                            sector_buffer);
                RootDirCnt++;
            }
        }
    } else  // 要查询的目录不是根目录
    {
        DIR_ENTRY Dir;
        off_t     offset_dir;

        if (find_root(fat16_ins, &Dir, path, &offset_dir))
            return -ENOENT;
        if (Dir.DIR_Attr == ATTR_ARCHIVE)
            return -ENOTDIR;

        WORD ClusterN;  // 当前读取的簇号
        WORD FatClusEntryVal;  // 该簇的 FAT 表项 (大部分情况下, 代表下一个簇的簇号,
                               // 请参考实验文档对FAT表项的说明)
        WORD FirstSectorofCluster;  // 该簇的第一个扇区号

        ClusterN = Dir.DIR_FstClusLO;  // 目录项中存储了我们要读取的第一个簇的簇号
        first_sector_by_cluster(fat16_ins, ClusterN, &FatClusEntryVal,
                                &FirstSectorofCluster, sector_buffer);

        /* Start searching the root's sub-directories starting from Dir */
        for (uint i = 1; Dir.DIR_Name[0] != 0x00; i++) {
            /*** BEGIN ***/
            memcpy(&Dir,
                   sector_buffer + ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR,
                   BYTES_PER_DIR);

            int is_empty   = (Dir.DIR_Name[0] == 0x00);
            int is_deleted = (Dir.DIR_Name[0] == 0xE5);
            int is_lfn     = (Dir.DIR_Attr == 0x0F);
            if (is_empty)
                break;
            if (!is_deleted && !is_lfn) {
                char *name = (char *)path_decode(Dir.DIR_Name);
                filler(buffer, name, NULL, 0);
                free(name);
            }
            /*** END ***/

            // 当前扇区的所有目录项已经读完.
            if (i % 16 == 0) {
                // 如果当前簇还有未读的扇区
                if (DirSecCnt < fat16_ins->Bpb.BPB_SecPerClus) {
                    /*** BEGIN ***/
                    sector_read(fat16_ins->fd, FirstSectorofCluster + DirSecCnt,
                                sector_buffer);
                    DirSecCnt++;
                    /*** END ***/
                } else  
                {
                    if (FatClusEntryVal == CLUSTER_END) {
                        break;
                    }
                    /*** BEGIN ***/
                    ClusterN = FatClusEntryVal;
                    first_sector_by_cluster(fat16_ins, ClusterN, &FatClusEntryVal,
                                            &FirstSectorofCluster, sector_buffer);
                    i         = 0;
                    DirSecCnt = 1;
                    /*** END ***/
                }
            }
        }
    }
    return 0;
}
```

### 读文件内容 `fat16_read()`
注释中将需要进行的步骤写出了, 这里直接复制到下面:
1. 由于文件是按簇组织的, 簇又是由扇区组成的, 要读取的偏移量可能在簇和扇区的中间, 所以本函数的关键就是读取范围的开始和结束的簇, 扇区, 扇区中的偏移, 为此, 你需要计算出：
    1. 要读取数据的在文件中的范围 (字节)：这很简单, `beginBytes = offset`, `endBytes = offset + size - 1`
    2. `beginBytes` 在文件中哪一个簇, `endBytes` 在文件中哪一个簇?
    3. `beginBytes` 在簇中第几个扇区, `endBytes` 在簇中第几个扇区?
    4. `beginBytes` 在扇区中哪个位置, `endBytes` 在扇区中哪个位置?
2. 计算出上述值后, 你需要通过 `find_root()` 找到文件对应的目录项, 并找到文件首个簇号. 这个步骤和 `readdir()` 中比较相似, 可以参考.
3. 找到首个簇号后, 通过 FAT 表依次遍历找到下一个簇的簇号, 直到 `beginBytes` 所在簇. 遍历簇的方法是通过 `fat_entry_by_cluster` 读取 FAT 表项 (也就是下一个簇号). 注意模仿 `readdir()` 处理文件结束. (哪一个簇号代表文件结束? `0xffff`)
4. 读取 `beginBytes` 对应的扇区, 然后循环读取簇和扇区, 同时填充 `buffer`, 直到 `endBytes` 所在的扇区.
5. 注意第一个和最后一个扇区中, 根据偏移量, 我们不一定需要扇区中所有数据, 请根据你在 1.4 中算出的值将正确的部分填入 `buffer`. 而中间所有扇区, 一定需要被全部读取.

在实际编写代码时, 需要考虑下面的几个情况:
- `beginBytes` 和 `endBytes` 在同一个扇区内
  - 这时只需要将一个扇区的对应区域填入 `buffer`
- `beginBytes` 和 `endBytes` 在同一个簇内
  - 这时要在一个簇内, 先读取第一个扇区的后半部分, 然后读取中间的所有扇区, 再读取最后一个扇区的前半部分
- `beginBytes` 和 `endBytes` 不在同一个簇内
  - 类似, 不过这次是先读取第一个簇的后半部分, 再读取中间的簇, 再读取最后一个簇的前半部分, 注意簇号不一定是连续的, 其结构类似一个链表.

```c
int fat16_read(const char *path, char *buffer, size_t size, off_t offset,
               struct fuse_file_info *fi)
{
    FAT16 *fat16_ins = get_fat16_ins();

    /*** BEGIN ***/
    BYTE      sector_buffer[BYTES_PER_SECTOR];
    DIR_ENTRY Dir;
    off_t     offset_dir;
    WORD      cluster_num;         // 当前读取的簇号
    WORD      fat_clus_entry_val;  // 该簇的 FAT 表项 (下一个簇号)
    WORD      first_sec_of_clus;   // 该簇的第一个扇区号
    if (find_root(fat16_ins, &Dir, path, &offset_dir))
        return 0;

    size_t begin_bytes = offset;
    size_t end_bytes   = offset + size - 1;
    if (end_bytes > Dir.DIR_FileSize)
        end_bytes = Dir.DIR_FileSize;

    WORD begin_cluster = begin_bytes / fat16_ins->ClusterSize;
    WORD end_cluster   = end_bytes / fat16_ins->ClusterSize;
    WORD begin_sector  = (begin_bytes % fat16_ins->ClusterSize) / BYTES_PER_SECTOR;
    WORD end_sector    = (end_bytes % fat16_ins->ClusterSize) / BYTES_PER_SECTOR;
    WORD begin_offset  = (begin_bytes % fat16_ins->ClusterSize) % BYTES_PER_SECTOR;
    WORD end_offset    = (end_bytes % fat16_ins->ClusterSize) % BYTES_PER_SECTOR;

    cluster_num = Dir.DIR_FstClusLO;  // 目录项中存储了我们要读取的第一个簇的簇号
    first_sector_by_cluster(fat16_ins, cluster_num, &fat_clus_entry_val,
                            &first_sec_of_clus, sector_buffer);

    WORD   cluster_cnt = 0;  // 记录读取的簇数
    WORD   sector_cnt  = 0;  // 记录读取的扇区数
    size_t read_size   = 0;  // 读取的字节数

    // 找到 begin_cluster
    while (cluster_cnt < begin_cluster) {
        if (fat_clus_entry_val == CLUSTER_END)
            return 0;
        cluster_num = fat_clus_entry_val;
        first_sector_by_cluster(fat16_ins, cluster_num, &fat_clus_entry_val,
                                &first_sec_of_clus, sector_buffer);
        cluster_cnt++;
    }

    // begin_cluster 和 end_cluster 为同一个簇
    if (begin_cluster == end_cluster) {
        if (begin_sector == end_sector) {
            sector_read(fat16_ins->fd, first_sec_of_clus + begin_sector,
                        sector_buffer);
            memcpy(buffer + read_size, sector_buffer + begin_offset,
                   end_offset - begin_offset + 1);
            read_size += end_offset - begin_offset + 1;
        } else {
            sector_read(fat16_ins->fd, first_sec_of_clus + begin_sector,
                        sector_buffer);
            memcpy(buffer + read_size, sector_buffer + begin_offset,
                   BYTES_PER_SECTOR - begin_offset);
            read_size += BYTES_PER_SECTOR - begin_offset;
            for (sector_cnt = begin_sector + 1; sector_cnt < end_sector;
                 sector_cnt++) {
                sector_read(fat16_ins->fd, first_sec_of_clus + sector_cnt,
                            sector_buffer);
                memcpy(buffer + read_size, sector_buffer, BYTES_PER_SECTOR);
                read_size += BYTES_PER_SECTOR;
            }
            sector_read(fat16_ins->fd, first_sec_of_clus + end_sector,
                        sector_buffer);
            memcpy(buffer + read_size, sector_buffer, end_offset + 1);
            read_size += end_offset + 1;
        }
    } else {
        // 读取第一个簇
        sector_read(fat16_ins->fd, first_sec_of_clus + begin_sector, sector_buffer);
        memcpy(buffer + read_size, sector_buffer + begin_offset,
               BYTES_PER_SECTOR - begin_offset);
        read_size += BYTES_PER_SECTOR - begin_offset;
        for (sector_cnt = begin_sector + 1;
             sector_cnt < fat16_ins->Bpb.BPB_SecPerClus; sector_cnt++)
        {
            sector_read(fat16_ins->fd, first_sec_of_clus + sector_cnt,
                        sector_buffer);
            memcpy(buffer + read_size, sector_buffer, BYTES_PER_SECTOR);
            read_size += BYTES_PER_SECTOR;
        }
        // 读取中间簇
        if (fat_clus_entry_val == CLUSTER_END)
            return read_size;
        cluster_num = fat_clus_entry_val;
        first_sector_by_cluster(fat16_ins, cluster_num, &fat_clus_entry_val,
                                &first_sec_of_clus, sector_buffer);
        cluster_cnt++;
        while (cluster_cnt < end_cluster) {
            for (sector_cnt = 0; sector_cnt < fat16_ins->Bpb.BPB_SecPerClus;
                 sector_cnt++) {
                sector_read(fat16_ins->fd, first_sec_of_clus + sector_cnt,
                            sector_buffer);
                memcpy(buffer + read_size, sector_buffer, BYTES_PER_SECTOR);
                read_size += BYTES_PER_SECTOR;
            }
            if (fat_clus_entry_val == CLUSTER_END)
                return read_size;
            cluster_num = fat_clus_entry_val;
            first_sector_by_cluster(fat16_ins, cluster_num, &fat_clus_entry_val,
                                    &first_sec_of_clus, sector_buffer);
            cluster_cnt++;
        }
        // 读取最后一个簇
        for (sector_cnt = 0; sector_cnt < end_sector; sector_cnt++) {
            sector_read(fat16_ins->fd, first_sec_of_clus + sector_cnt,
                        sector_buffer);
            memcpy(buffer + read_size, sector_buffer, BYTES_PER_SECTOR);
            read_size += BYTES_PER_SECTOR;
        }
        sector_read(fat16_ins->fd, first_sec_of_clus + end_sector, sector_buffer);
        memcpy(buffer + read_size, sector_buffer, end_offset + 1);
        read_size += end_offset + 1;
    }
    return read_size;
    /*** END ***/
}
```

## Task 2: FAT16 文件系统创建/删除文件操作
### 创建文件 `fat16_mknod()` & `dir_entry_create()`
#### `fat16_mknod()`
`fat16_mknod()` 的代码结构与 [`fat16_readdir()`](#读目录-fat16_readdir) 类似, 需要创建文件的文件名为 `paths[-1]` (例如 `TEST    C  `). 
- 在对应目录内找到空闲目录项或被删除目录项时, 记录相应信息并设置 `flag = 1`, 跳出循环; 
- 找到同名文件时 (使用 `strncmp()` 函数), 设置 `flag = 0` 并跳出循环;
- 若 `flag == 1`, 则使用 `dir_entry_create()` 创建相应的目录项.

```c
int fat16_mknod(const char *path, mode_t mode, dev_t devNum)
{
    FAT16 *fat16_ins = get_fat16_ins();

    // 查找需要创建文件的父目录路径
    int          pathDepth;
    char       **paths    = path_split((char *)path, &pathDepth);
    char        *copyPath = strdup(path);
    const char **orgPaths = (const char **)org_path_split(copyPath);
    char        *prtPath  = get_prt_path(path, orgPaths, pathDepth);

    BYTE  sector_buffer[BYTES_PER_SECTOR];
    DWORD sectorNum;
    int   offset, i, findFlag = 0, RootDirCnt = 1, DirSecCnt = 1;
    WORD  ClusterN, FatClusEntryVal, FirstSectorofCluster;

    /* If parent directory is root */
    if (strcmp(prtPath, "/") == 0) {
        /*** BEGIN ***/
        DIR_ENTRY Root;
        sector_read(fat16_ins->fd, fat16_ins->FirstRootDirSecNum, sector_buffer);
        for (uint i = 1; i <= fat16_ins->Bpb.BPB_RootEntCnt; i++) {
            memcpy(&Root,
                   sector_buffer + ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR,
                   BYTES_PER_DIR);

            int is_empty   = (Root.DIR_Name[0] == 0x00);
            int is_deleted = (Root.DIR_Name[0] == 0xE5);
            int is_lfn     = (Root.DIR_Attr == 0x0F);
            if (is_empty || is_deleted) {
                sectorNum = fat16_ins->FirstRootDirSecNum + RootDirCnt - 1;
                offset    = ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR;
                findFlag  = 1;
                break;
            } else if (!is_lfn) {
                if (strncmp((char *)Root.DIR_Name, paths[pathDepth - 1], 11) == 0) {
                    findFlag = 0;
                    break;
                }
            }

            // 当前扇区所有条目已经读取完毕, 将下一个扇区读入 sector_buffer
            if (i % 16 == 0 && i != fat16_ins->Bpb.BPB_RootEntCnt) {
                sector_read(fat16_ins->fd,
                            fat16_ins->FirstRootDirSecNum + RootDirCnt,
                            sector_buffer);
                RootDirCnt++;
            }
        }
        /*** END ***/
    }
    /* Else if parent directory is sub-directory */
    else
    {
        /*** BEGIN ***/
        DIR_ENTRY Dir;
        off_t     offset_dir;

        if (find_root(fat16_ins, &Dir, prtPath, &offset_dir))
            return -ENOENT;
        if (Dir.DIR_Attr == ATTR_ARCHIVE)
            return -ENOTDIR;

        ClusterN = Dir.DIR_FstClusLO;
        first_sector_by_cluster(fat16_ins, ClusterN, &FatClusEntryVal,
                                &FirstSectorofCluster, sector_buffer);

        for (uint i = 1; Dir.DIR_Name[0] != 0x00; i++) {
            memcpy(&Dir,
                   sector_buffer + ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR,
                   BYTES_PER_DIR);

            int is_empty   = (Dir.DIR_Name[0] == 0x00);
            int is_deleted = (Dir.DIR_Name[0] == 0xE5);
            int is_lfn     = (Dir.DIR_Attr == 0x0F);
            if (is_empty || is_deleted) {
                sectorNum = FirstSectorofCluster + DirSecCnt - 1;
                offset    = ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR;
                findFlag  = 1;
                break;
            } else if (!is_lfn) {
                if (strncmp((char *)Dir.DIR_Name, paths[pathDepth - 1], 11) == 0) {
                    findFlag = 0;
                    break;
                }
            }

            // 当前扇区的所有目录项已经读完.
            if (i % 16 == 0) {
                // 如果当前簇还有未读的扇区
                if (DirSecCnt < fat16_ins->Bpb.BPB_SecPerClus) {
                    sector_read(fat16_ins->fd, FirstSectorofCluster + DirSecCnt,
                                sector_buffer);
                    DirSecCnt++;
                } else {
                    if (FatClusEntryVal == CLUSTER_END) {
                        break;
                    }
                    ClusterN = FatClusEntryVal;
                    first_sector_by_cluster(fat16_ins, ClusterN, &FatClusEntryVal,
                                            &FirstSectorofCluster, sector_buffer);
                    i         = 0;
                    DirSecCnt = 1;
                }
            }
        }
        /*** END ***/
    }

    /* Add the DIR ENTRY */
    if (findFlag == 1) {
        // TODO: 正确调用 dir_entry_create 创建目录项
        /*** BEGIN ***/
        dir_entry_create(fat16_ins, sectorNum, offset, paths[pathDepth - 1],
                         ATTR_ARCHIVE, CLUSTER_END, 0);
        /*** END ***/
    }
    return 0;
}
```

#### `dir_entry_create()`
`dir_entry_create()` 函数需要将相应的值填入目录项, 再写入扇区, 
1. 按照目录项结构, 将信息填入 `entry_info` 中;
2. 使用 `sector_read()` 读取对应扇区, 然后替换其中的一个目录项, 再使用 `sector_write()` 写入文件系统.

其中目录项结构为

|        Name        | Offset | Length |                               Detail                                |
| :----------------: | :----: | :----: | :-----------------------------------------------------------------: |
|     `DIR_Name`     | `0x00` |   11   | 文件名. 前 8 个字节为文件名, 后 3 个为拓展名. 第一位取值有特殊含义. |
|     `DIR_Attr`     | `0x0b` |   1    |             文件属性. `0x10` 表示目录, `0x20` 表示文件.             |
|    `DIR_NTRes`     | `0x0c` |   1    |                                保留                                 |
| `DIR_CrtTimeTenth` | `0x0D` |   1    |              保留 (FAT32 中用作创建时间, 精确到 10ms)               |
|   `DIR_CrtTime`    | `0x0E` |   2    |               保留 (FAT32 中用作创建时间, 精确到 2s)                |
|   `DIR_CrtDate`    | `0x10` |   2    |                     保留 (FAT32 中用作创建日期)                     |
|  `DIR_LstAccDate`  | `0x12` |   2    |                   保留 (FAT32 中用作最近访问日期)                   |
|  `DIR_FstClusHI`   | `0x14` |   2    |                保留 (FAT32 用作第一个簇的两个高字节)                |
|   `DIR_WrtTime`    | `0x16` |   2    |                          文件最近修改时间                           |
|   `DIR_WrtDate`    | `0x18` |   2    |                          文件最近修改日期                           |
|  `DIR_FstClusLO`   | `0x1A` |   2    |              文件首簇号 (FAT32用作第一个簇的两个低字节)              |
|   `DIR_FileSize`   | `0x1C` |   4    |                              文件大小                               |

我们可以在创建时将 `DIR_FstClusLO` 设为 `0xffff`, `DIR_FileSize` 设为 0.

```c
int dir_entry_create(FAT16 *fat16_ins, int sectorNum, int offset, char *Name,
                     BYTE attr, WORD firstClusterNum, DWORD fileSize)
{
    /* Create memory buffer to store entry info */
    // 先在 buffer 中写好表项的信息, 最后通过一次 IO 写入到磁盘中
    BYTE *entry_info = malloc(BYTES_PER_DIR * sizeof(BYTE));

    // 为新表项填入文件名和文件属性
    /*** BEGIN ***/
    memset(entry_info, 0, BYTES_PER_DIR);
    memcpy(entry_info, Name, 11 * sizeof(BYTE));
    entry_info[11] = attr;
    /*** END ***/

    /**
     * 关于时间信息部分的工作我们本次实验不要求
     * 代码已经给出, 可以参考实验文档自行理解
     **/
    time_t timer_s;
    time(&timer_s);
    struct tm *time_ptr = localtime(&timer_s);
    int        value;

    /* Unused */
    memset(entry_info + 12, 0, 10 * sizeof(BYTE));

    /* File update time */
    /* 时间部分可以阅读实验文档 */
    value =
        time_ptr->tm_sec / 2 + (time_ptr->tm_min << 5) + (time_ptr->tm_hour << 11);
    memcpy(entry_info + 22, &value, 2 * sizeof(BYTE));

    /* File update date */
    value = time_ptr->tm_mday + (time_ptr->tm_mon << 5)
          + ((time_ptr->tm_year - 80) << 9);
    memcpy(entry_info + 24, &value, 2 * sizeof(BYTE));

    // 为新表项填入文件首簇号与文件大小
    /*** BEGIN ***/
    memcpy(entry_info + 26, &firstClusterNum, 2 * sizeof(BYTE));
    memcpy(entry_info + 28, &fileSize, 4 * sizeof(BYTE));
    /*** END ***/

    // 将创建好的新表项信息写入到磁
    /*** BEGIN ***/
    BYTE sector_buffer[BYTES_PER_SECTOR];
    sector_read(fat16_ins->fd, sectorNum, sector_buffer);
    memcpy(sector_buffer + offset, entry_info, BYTES_PER_DIR * sizeof(BYTE));
    sector_write(fat16_ins->fd, sectorNum, sector_buffer);
    /*** END ***/

    free(entry_info);
    return 0;
}
```

### 删除文件 `free_cluster()` & `fat16_unlink()`
#### `free_cluster()`
求出两个 FAT 表的对应表项位置, 然后调用 `sector_read()` 读取相应扇区, 修改表项后 `sector_write()` 写回即可.
```c
int free_cluster(FAT16 *fat16_ins, int ClusterNum)
{
    BYTE sector_buffer[BYTES_PER_SECTOR];
    WORD FATClusEntryval, FirstSectorofCluster;
    first_sector_by_cluster(fat16_ins, ClusterNum, &FATClusEntryval,
                            &FirstSectorofCluster, sector_buffer);

    FILE *fd = fat16_ins->fd;
    
    /*** BEGIN ***/
    WORD  fat_offset    = (WORD)ClusterNum * 2;
    DWORD fat_1_sec_num = fat16_ins->Bpb.BPB_RsvdSecCnt 
                        + fat_offset / fat16_ins->Bpb.BPB_BytsPerSec;
    DWORD fat_2_sec_num = fat16_ins->Bpb.BPB_RsvdSecCnt + fat16_ins->Bpb.BPB_FATSz16
                        + fat_offset / fat16_ins->Bpb.BPB_BytsPerSec;
    WORD fat_1_offset   = fat_offset % fat16_ins->Bpb.BPB_BytsPerSec;
    WORD fat_2_offset   = fat_offset % fat16_ins->Bpb.BPB_BytsPerSec;

    sector_read(fd, fat_1_sec_num, sector_buffer);
    sector_buffer[fat_1_offset] = 0x00;
    sector_write(fd, fat_1_sec_num, sector_buffer);

    sector_read(fd, fat_2_sec_num, sector_buffer);
    sector_buffer[fat_2_offset] = 0x00;
    sector_write(fd, fat_2_sec_num, sector_buffer);
    /*** END ***/

    return FATClusEntryval;
}
```

#### `fat16_unlink()`
与 [`fat16_mknod()`](#fat16mknod) 类似, 不过这次是找到同名文件才保存信息并设置 `flag = 1`.
```c
int fat16_unlink(const char *path)
{
    FAT16 *fat16_ins = get_fat16_ins();

    DIR_ENTRY Dir;
    off_t     offset_dir;
    // 释放使用过的簇
    if (find_root(fat16_ins, &Dir, path, &offset_dir) == 1) {
        return -ENOENT;
    }
    if (Dir.DIR_Attr == ATTR_DIRECTORY) {
        return -EISDIR;
    }

    /*** BEGIN ***/
    WORD iter_clus_num = Dir.DIR_FstClusLO;
    while (iter_clus_num != 0xffff) {
        iter_clus_num = free_cluster(fat16_ins, iter_clus_num);
    }
    /*** END ***/

    // 查找需要删除文件的父目录路径
    int          pathDepth;
    char       **paths    = path_split((char *)path, &pathDepth);
    char        *copyPath = strdup(path);
    const char **orgPaths = (const char **)org_path_split(copyPath);
    char        *prtPath  = get_prt_path(path, orgPaths, pathDepth);

    BYTE  sector_buffer[BYTES_PER_SECTOR];
    DWORD sectorNum;
    int   offset, i, findFlag = 0, RootDirCnt = 1, DirSecCnt = 1;
    WORD  ClusterN, FatClusEntryVal, FirstSectorofCluster;

    /* If parent directory is root */
    if (strcmp(prtPath, "/") == 0) {
        /*** BEGIN ***/
        DIR_ENTRY Root;
        sector_read(fat16_ins->fd, fat16_ins->FirstRootDirSecNum, sector_buffer);
        for (uint i = 1; i <= fat16_ins->Bpb.BPB_RootEntCnt; i++) {
            memcpy(&Root,
                   sector_buffer + ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR,
                   BYTES_PER_DIR);

            int is_empty   = (Root.DIR_Name[0] == 0x00);
            int is_deleted = (Root.DIR_Name[0] == 0xE5);
            int is_lfn     = (Root.DIR_Attr == 0x0F);

            if (is_empty) {
                break;
            } else if (!is_deleted && !is_lfn) {
                if (strncmp((char *)Root.DIR_Name, paths[pathDepth - 1], 11) == 0) {
                    sectorNum = fat16_ins->FirstRootDirSecNum + RootDirCnt - 1;
                    offset    = ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR;
                    findFlag  = 1;
                    break;
                }
            }

            // 当前扇区所有条目已经读取完毕, 将下一个扇区读入 sector_buffer
            if (i % 16 == 0 && i != fat16_ins->Bpb.BPB_RootEntCnt) {
                sector_read(fat16_ins->fd,
                            fat16_ins->FirstRootDirSecNum + RootDirCnt,
                            sector_buffer);
                RootDirCnt++;
            }
        }
        /*** END ***/
    } else /* Else if parent directory is sub-directory */ {
        /*** BEGIN ***/
        if (find_root(fat16_ins, &Dir, prtPath, &offset_dir))
            return -ENOENT;
        if (Dir.DIR_Attr == ATTR_ARCHIVE)
            return -ENOTDIR;
        
        ClusterN = Dir.DIR_FstClusLO;
        first_sector_by_cluster(fat16_ins, ClusterN, &FatClusEntryVal,
                                &FirstSectorofCluster, sector_buffer);

        /* Start searching the root's sub-directories starting from Dir */
        for (uint i = 1; Dir.DIR_Name[0] != 0x00; i++) {
            memcpy(&Dir,
                   sector_buffer + ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR,
                   BYTES_PER_DIR);

            int is_empty   = (Dir.DIR_Name[0] == 0x00);
            int is_deleted = (Dir.DIR_Name[0] == 0xE5);
            int is_lfn     = (Dir.DIR_Attr == 0x0F);

            if (is_empty) {
                break;
            } else if (!is_deleted && !is_lfn) {
                if (strncmp((char *)Dir.DIR_Name, paths[pathDepth - 1], 11) == 0) {
                    sectorNum = FirstSectorofCluster + DirSecCnt - 1;
                    offset    = ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR;
                    findFlag  = 1;
                    break;
                }
            }

            // 当前扇区的所有目录项已经读完.
            if (i % 16 == 0) {
                // 如果当前簇还有未读的扇区
                if (DirSecCnt < fat16_ins->Bpb.BPB_SecPerClus) {
                    sector_read(fat16_ins->fd, FirstSectorofCluster + DirSecCnt,
                                sector_buffer);
                    DirSecCnt++;
                } else {
                    if (FatClusEntryVal == CLUSTER_END) {
                        break;
                    }
                    ClusterN = FatClusEntryVal;
                    first_sector_by_cluster(fat16_ins, ClusterN, &FatClusEntryVal,
                                            &FirstSectorofCluster, sector_buffer);
                    i         = 0;
                    DirSecCnt = 1;
                }
            }
        }
        /*** END ***/
    }

    /* Update file entry, change its first byte of file name to 0xe5 */
    if (findFlag == 1) {
        /*** BEGIN ***/
        paths[pathDepth - 1][0] = 0xe5;
        dir_entry_create(fat16_ins, sectorNum, offset, paths[pathDepth - 1],
                         ATTR_ARCHIVE, CLUSTER_END, 0);
        /*** END ***/
    }
    return 0;
}
```
# Part 2 (Complete in 5 Hour)

## 前置任务 `alloc_clusters()` & `write_fat_entry()`
### 空闲簇分配 `alloc_clusters()`
函数实现思路与试验文档中的相同, 即
1. 扫描 FAT 表, 找到 n 个空闲的簇（空闲簇的 FAT 表项为 `0x0000` ）
2. 若找不到 n 个空闲簇, 直接返回 `CLUSTER_END` 作为错误提示. 注意, 找不到 n 个簇时, 不应修改任何 FAT 表项. 
3. 依次清零 n 个簇, 这需要将 0 写入每个簇的所有扇区
4. 依次修改 n 个簇的 FAT 表项, 将每个簇通过 FAT 表项指向下一个簇, 第 n 个簇的 FAT 表项应该指向 `CLUSTER_END`
5. 返回第 1 个簇的簇号

```c
WORD alloc_clusters(FAT16 *fat16_ins, uint32_t n)
{
    if (n == 0)
        return CLUSTER_END;

    WORD *clusters  = malloc((n + 1) * sizeof(WORD));
    uint  allocated = 0;  // 已找到的空闲簇个数

    // 扫描 FAT 表, 找到 n 个空闲的簇, 存入 cluster 数组.
    // 注意此时不需要修改对应的 FAT 表项
    /*** BEGIN ***/
    BYTE sectorBuffer[BYTES_PER_SECTOR];
    uint firstFatSecNum = fat16_ins->Bpb.BPB_RsvdSecCnt;
    WORD clusEntry;
    for (uint32_t i = 0; i < fat16_ins->Bpb.BPB_FATSz16; i++) {
        sector_read(fat16_ins->fd, firstFatSecNum + i, sectorBuffer);
        for (uint32_t j = 0; j < fat16_ins->Bpb.BPB_BytsPerSec; j += 2) {
            memcpy(&clusEntry, &sectorBuffer[j], sizeof(WORD));
            if (clusEntry == CLUSTER_FREE) {
                clusters[allocated] = (i * fat16_ins->Bpb.BPB_BytsPerSec + j) / 2;
                allocated++;
                if (allocated == n) {
                    clusters[allocated] = CLUSTER_END;
                    break;
                }
            }
        }
        if (allocated == n)
            break;
    }
    /*** END ***/

    if (allocated != n) {  // 找不到 n 个簇, 分配失败
        free(clusters);
        return CLUSTER_END;
    }

    // 找到了 n 个空闲簇, 将 CLUSTER_END 加至末尾.
    clusters[n] = CLUSTER_END;

    // 修改 clusters 中存储的 N 个簇对应的 FAT 表项,
    // 将每个簇与下一个簇连接在一起. 同时清零每一个新分配的簇.
    /*** BEGIN ***/
    BYTE emptySector[BYTES_PER_SECTOR];
    WORD fatClusEntryVal;
    WORD firstSectorOfCluster;
    memset(emptySector, 0, BYTES_PER_SECTOR);
    for (uint i = 0; i < n; i++) {
        write_fat_entry(fat16_ins, clusters[i], clusters[i + 1]);
        first_sector_by_cluster(fat16_ins, clusters[i], &fatClusEntryVal,
                                &firstSectorOfCluster, sectorBuffer);
        for (uint j = 0; j < fat16_ins->Bpb.BPB_SecPerClus; j++) {
            sector_write(fat16_ins->fd, firstSectorOfCluster + j, emptySector);
        }
    }
    /*** END ***/

    // 返回首个分配的簇
    WORD first_cluster = clusters[0];
    free(clusters);
    return first_cluster;
}
```
### 修改 FAT 表项 `write_fat_entry()`
找到对应扇区然后写回即可, 注意要填写多个表, 类似 [`free_cluster()`](#free_cluster), 这里不再赘述
```c
int write_fat_entry(FAT16 *fat16_ins, WORD clusterN, WORD data)
{
    // 这个函数逻辑与 fat_entry_by_cluster 函数类似,
    // 但这个函数需要修改对应值并写回 FAT 表中
    BYTE SectorBuffer[BYTES_PER_SECTOR];
    uint firstFatSecNum;  // 第一个 FAT 表开始的扇区号
    uint clusterOffset;   // clusterN 这个簇对应的表项, 在每个 FAT 表项的哪个偏移量
    uint clusterSec;      // clusterN 这个簇对应的表项, 在每个 FAT 表中的第几个扇区
    uint secOffset;       // clusterN 这个簇对应的表项, 在所在扇区的哪个偏移量
    /*** BEGIN ***/
    firstFatSecNum = fat16_ins->Bpb.BPB_RsvdSecCnt;
    clusterOffset  = clusterN * 2;
    clusterSec     = clusterOffset / fat16_ins->Bpb.BPB_BytsPerSec;
    secOffset      = clusterOffset % fat16_ins->Bpb.BPB_BytsPerSec;
    /*** END ***/
    // 对系统中每个 FAT 表都进行写入
    for (uint i = 0; i < fat16_ins->Bpb.BPB_NumFATS; i++) {
        /*** BEGIN ***/
        uint secNum = i * fat16_ins->Bpb.BPB_FATSz16 + firstFatSecNum + clusterSec;
        sector_read(fat16_ins->fd, secNum, SectorBuffer);
        memcpy(SectorBuffer + secOffset, &data, sizeof(WORD));
        sector_write(fat16_ins->fd, secNum, SectorBuffer);
        /*** END ***/
    }
    return 0;
}
```

## Task 3: FAT16 文件系统创建/删除文件夹操作 `fat16_mkdir()` & `fat16_rmdir()`
### 创建文件夹 `fat16_mkdir()`
这里的实现思路大体与 [`fat16_mknod()`](#fat16_mknod) 类似, 只不过在最后一步除了填入 entry 外还要在簇中插入 `.` 和 `..` 两个目录项, 因为不涉及目录链接操作, 所以不需要将它们链接到相应目录. 目录的大小设为 0 即可.
```c
int fat16_mkdir(const char *path, mode_t mode)
{
    FAT16 *fat16_ins = get_fat16_ins_fix();

    int findFlag  = 0;  // 是否找到空闲的目录项
    int sectorNum = 0;  // 找到的空闲目录项所在扇区号
    int offset    = 0;  // 找到的空闲目录项在扇区中的偏移量

    /*** BEGIN ***/
    // 查找需要创建文件夹的父目录路径
    int          pathDepth;
    char       **paths    = path_split((char *)path, &pathDepth);
    char        *copyPath = strdup(path);
    const char **orgPaths = (const char **)org_path_split(copyPath);
    char        *prtPath  = get_prt_path(path, orgPaths, pathDepth);

    BYTE      sectorBuffer[BYTES_PER_SECTOR];
    DIR_ENTRY root;
    DIR_ENTRY dir;
    off_t     dirOffset;
    int       rootDirCnt = 1;
    int       dirSecCnt  = 1;
    WORD      clusterN;              // 当前读取的簇号
    WORD      fatClusEntryVal;       // 下一个簇的簇号
    WORD      firstSectorOfCluster;  // 该簇的第一个扇区号

    /* If parent directory is root */
    if (strcmp(prtPath, "/") == 0) {
        sector_read(fat16_ins->fd, fat16_ins->FirstRootDirSecNum, sectorBuffer);
        for (uint i = 1; i <= fat16_ins->Bpb.BPB_RootEntCnt; i++) {
            memcpy(&root,
                   sectorBuffer + ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR,
                   BYTES_PER_DIR);

            int isEmpty   = (root.DIR_Name[0] == 0x00);
            int isDeleted = (root.DIR_Name[0] == 0xe5);
            int isLFN     = (root.DIR_Attr == 0x0f);

            if (!isEmpty || !isDeleted) {
                sectorNum = fat16_ins->FirstRootDirSecNum + rootDirCnt - 1;
                offset    = ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR;
                findFlag  = 1;
                break;
            } else if (!isLFN) {
                if (strncmp((char *)root.DIR_Name, paths[pathDepth - 1], 11) == 0) {
                    findFlag = 0;
                    break;
                }
            }
            // 当前扇区所有条目已经读取完毕, 将下一个扇区读入 sector_buffer
            if (i % 16 == 0 && i != fat16_ins->Bpb.BPB_RootEntCnt) {
                sector_read(fat16_ins->fd,
                            fat16_ins->FirstRootDirSecNum + rootDirCnt,
                            sectorBuffer);
                rootDirCnt++;
            }
        }
    } else { /* Else if parent directory is sub-directory */
        if (find_root(fat16_ins, &dir, prtPath, &dirOffset))
            return -ENOENT;
        else if (dir.DIR_Attr == ATTR_ARCHIVE)
            return -ENOTDIR;

        clusterN = dir.DIR_FstClusLO;
        first_sector_by_cluster(fat16_ins, clusterN, &fatClusEntryVal,
                                &firstSectorOfCluster, sectorBuffer);

        for (uint i = 1; dir.DIR_Name[0] != 0x00; i++) {
            memcpy(&dir, sectorBuffer + ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR,
                   BYTES_PER_DIR);

            int isEmpty   = (dir.DIR_Name[0] == 0x00);
            int isDeleted = (dir.DIR_Name[0] == 0xe5);
            int isLFN     = (dir.DIR_Attr == 0x0f);

            if (!isEmpty || !isDeleted) {
                sectorNum = firstSectorOfCluster + rootDirCnt - 1;
                offset    = ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR;
                findFlag  = 1;
                break;
            } else if (!isLFN) {
                if (strncmp((char *)dir.DIR_Name, paths[pathDepth - 1], 11) == 0) {
                    findFlag = 0;
                    break;
                }
            }
            // 当前扇区的所有目录项已经读完.
            if (i % 16 == 0) {
                // 如果当前簇还有未读的扇区
                if (dirSecCnt < fat16_ins->Bpb.BPB_SecPerClus) {
                    sector_read(fat16_ins->fd, firstSectorOfCluster + dirSecCnt,
                                sectorBuffer);
                    dirSecCnt++;
                } else {
                    if (fatClusEntryVal == CLUSTER_END)
                        break;

                    clusterN = fatClusEntryVal;
                    first_sector_by_cluster(fat16_ins, clusterN, &fatClusEntryVal,
                                            &firstSectorOfCluster, sectorBuffer);
                    i         = 0;
                    dirSecCnt = 1;
                }
            }
        }
    }

    /*** END ***/
    // 在父目录的目录项中添加新建的目录. 同时, 为新目录分配一个簇, 并在这个簇中创建两个
    // 目录项, 分别指向. 和 .., 目录的文件大小设置为 0 即可.
    if (findFlag == 1) {
        /*** BEGIN ***/
        WORD allocatedClus = alloc_clusters(fat16_ins, 1);
        if (allocatedClus == CLUSTER_END)
            return -ENOSPC;
        // 填 "." 和 ".." 目录项
        first_sector_by_cluster(fat16_ins, allocatedClus, &fatClusEntryVal,
                                &firstSectorOfCluster, sectorBuffer);
        dir_entry_create(fat16_ins, firstSectorOfCluster, 0, ".          ",
                         ATTR_DIRECTORY, allocatedClus, 0);
        dir_entry_create(fat16_ins, firstSectorOfCluster, BYTES_PER_DIR,
                         "..         ", ATTR_DIRECTORY, CLUSTER_END, 0);
        // 在父目录中添加新建的目录
        dir_entry_create(fat16_ins, sectorNum, offset, paths[pathDepth - 1],
                         ATTR_DIRECTORY, allocatedClus, 0);
        /*** END ***/
    }
    return 0;
}
```

### 删除文件夹 `fat16_rmdir()`
1. 找到目录, 确认目录为空. 
2. 释放目录占用的所有簇. 
3. 删除目录在父目录中的目录项. 
实现思路中的第一步和 [`fat16_readdir()`](#读目录-fat16_readdir) 有一些相似, 而第 2, 3 步则类似 [`fat16_unlink()`](#fat16_unlink). 可以参考 Part1 中的这两个函数来实现文件夹删除. 
```c
void dir_entry_delete(FAT16 *fat16_ins, off_t offset)
{
    BYTE buffer[BYTES_PER_SECTOR];
    // 删除目录项, 将镜像文件 offset 处的目录项第一个字节设置为 0xe5 即可.
    /*** BEGIN ***/
    sector_read(fat16_ins->fd, offset / fat16_ins->Bpb.BPB_BytsPerSec, buffer);
    buffer[offset % fat16_ins->Bpb.BPB_BytsPerSec] = 0xe5;
    sector_write(fat16_ins->fd, offset / fat16_ins->Bpb.BPB_BytsPerSec, buffer);
    /*** END ***/
}

void dir_entry_write(FAT16 *fat16_ins, off_t offset, const DIR_ENTRY *Dir)
{
    BYTE buffer[BYTES_PER_SECTOR];
    // 修改目录项, 将整个 Dir 写入 offset 所在的位置.
    /*** BEGIN ***/
    sector_read(fat16_ins->fd, offset / fat16_ins->Bpb.BPB_BytsPerSec, buffer);
    memcpy(buffer + (offset % fat16_ins->Bpb.BPB_BytsPerSec), Dir,
           sizeof(DIR_ENTRY));
    sector_write(fat16_ins->fd, offset / fat16_ins->Bpb.BPB_BytsPerSec, buffer);
    /*** END ***/
}

int fat16_rmdir(const char *path)
{
    FAT16 *fat16_ins = get_fat16_ins_fix();

    if (strcmp(path, "/") == 0) {
        return -EBUSY;  // 无法删除根目录, 根目录是挂载点 (可参考 `man 2 rmdir`)
    }

    DIR_ENTRY dir;
    DIR_ENTRY curDir;
    off_t     offset;

    if (find_root(fat16_ins, &dir, path, &offset)) {
        return -ENOENT;  // 路径不存在
    }
    if (dir.DIR_Attr != ATTR_DIRECTORY) {
        return ENOTDIR;  // 路径不是目录
    }

    // 检查目录是否为空, 如果目录不为空, 直接返回 -ENOTEMPTY. 
    // 注意空目录也可能有 "." 和 ".." 两个子目录.
    /*** BEGIN ***/
    BYTE sectorBuffer[BYTES_PER_SECTOR];
    WORD clusterN;              // 当前读取的簇号
    WORD fatClusEntryVal;       // 下一个簇的簇号
    WORD firstSectorOfCluster;  // 该簇的第一个扇区号
    int  dirSecCnt = 1;

    clusterN = dir.DIR_FstClusLO;  // 目录项中存储了我们要读取的第一个簇的簇号
    first_sector_by_cluster(fat16_ins, clusterN, &fatClusEntryVal,
                            &firstSectorOfCluster, sectorBuffer);

    for (uint i = 1; curDir.DIR_Name[0] != 0x00; i++) {
        memcpy(&curDir, sectorBuffer + ((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR,
               BYTES_PER_DIR);

        int isEmpty   = (curDir.DIR_Name[0] == 0x00);
        int isDeleted = (curDir.DIR_Name[0] == 0xe5);
        int isLFN     = (curDir.DIR_Attr == 0x0f);

        if (isEmpty) {
            break;
        }
        if (!isDeleted && !isLFN) {
            if (strncmp((char *)curDir.DIR_Name, ".          ", 11) != 0
                && strncmp((char *)curDir.DIR_Name, "..         ", 11) != 0)
            {
                return -ENOTEMPTY;
            }
        }

        // 当前扇区的所有目录项已经读完.
        if (i % 16 == 0) {
            // 如果当前簇还有未读的扇区
            if (dirSecCnt < fat16_ins->Bpb.BPB_SecPerClus) {
                sector_read(fat16_ins->fd, firstSectorOfCluster + dirSecCnt,
                            sectorBuffer);
                dirSecCnt++;
            } else {
                if (fatClusEntryVal == CLUSTER_END)
                    break;

                clusterN = fatClusEntryVal;
                first_sector_by_cluster(fat16_ins, clusterN, &fatClusEntryVal,
                                        &firstSectorOfCluster, sectorBuffer);
                i         = 0;
                dirSecCnt = 1;
            }
        }
    }
    /*** END ***/

    // 已确认目录项为空, 释放目录占用的簇
    /*** BEGIN ***/
    WORD iterClusNum = dir.DIR_FstClusLO;
    while (iterClusNum != CLUSTER_END)
        iterClusNum = free_cluster(fat16_ins, iterClusNum);
    /*** END ***/

    // 删除父目录中的目录项
    /*** BEGIN ***/
    dir_entry_delete(fat16_ins, offset);
    /*** END ***/

    return 0;
}
```

## FAT16 文件系统写操作
这一部分在注释中写的很详细, 这里直接放代码

```c
/**
 * @brief 将 data 中的数据写入编号为 clusterN 的簇的 offset 位置.
 *        注意 size+offset <= 簇大小
 *
 * @param fat16_ins 文件系统指针
 * @param clusterN  要写入数据的块号
 * @param data      要写入的数据
 * @param size      要写入数据的大小 (字节)
 * @param offset    要写入簇的偏移量
 * @return size_t   成功写入的字节数
 */
size_t write_to_cluster_at_offset(FAT16 *fat16_ins, WORD clusterN, off_t offset,
                                  const BYTE *data, size_t size)
{
    assert(offset + size <= fat16_ins->ClusterSize);  // offset + size 必须小于簇大小
    BYTE sectorBuffer[BYTES_PER_SECTOR];
    /** TODO: 将数据写入簇对应的偏移量上. 你需要找到第一个需要写入的扇区,
     *        和要写入的偏移量, 然后依次写入后续每个扇区, 直到所有数据都写入完成.
     * 注意, offset 对应的首个扇区和 offset+size 对应的最后一个扇区
     * 都可能只需要写入一部分. 所以应该先将扇区读出, 修改要写入的部分,
     * 再写回整个扇区.
     */
    /*** BEGIN ***/
    WORD fatClusEntryVal;       // 下一个簇的簇号
    WORD firstSectorOfCluster;  // 该簇的第一个扇区号
    first_sector_by_cluster(fat16_ins, clusterN, &fatClusEntryVal,
                            &firstSectorOfCluster, sectorBuffer);

    WORD beginSector = firstSectorOfCluster + offset / fat16_ins->Bpb.BPB_BytsPerSec;
    WORD endSector =
        firstSectorOfCluster + (offset + size) / fat16_ins->Bpb.BPB_BytsPerSec;

    off_t  beginOffset = offset % fat16_ins->Bpb.BPB_BytsPerSec;
    off_t  endOffset   = (offset + size) % fat16_ins->Bpb.BPB_BytsPerSec;
    size_t writeSize   = 0;

    if (beginSector == endSector) {
        sector_read(fat16_ins->fd, beginSector, sectorBuffer);
        memcpy(sectorBuffer + beginOffset, data, size);
        sector_write(fat16_ins->fd, beginSector, sectorBuffer);
        writeSize += size;
    } else {
        // 写入第一个扇区
        sector_read(fat16_ins->fd, beginSector, sectorBuffer);
        memcpy(sectorBuffer + beginOffset, data,
               fat16_ins->Bpb.BPB_BytsPerSec - beginOffset);
        sector_write(fat16_ins->fd, beginSector, sectorBuffer);
        writeSize += fat16_ins->Bpb.BPB_BytsPerSec - beginOffset;
        // 写入中间扇区
        for (WORD sectorCnt = beginSector + 1; sectorCnt < endSector; sectorCnt++) {
            sector_read(fat16_ins->fd, sectorCnt, sectorBuffer);
            memcpy(sectorBuffer, data + writeSize, fat16_ins->Bpb.BPB_BytsPerSec);
            sector_write(fat16_ins->fd, sectorCnt, sectorBuffer);
            writeSize += fat16_ins->Bpb.BPB_BytsPerSec;
        }
        // 写入最后一个扇区
        sector_read(fat16_ins->fd, endSector, sectorBuffer);
        memcpy(sectorBuffer, data + writeSize, endOffset);
        sector_write(fat16_ins->fd, endSector, sectorBuffer);
        writeSize += endOffset;
    }
    /*** END ***/
    return writeSize;
}

/**
 * @brief 查找文件最末尾的一个簇, 同时计算文件当前簇数, 如果文件没有任何簇, 返回
 *        CLUSTER_END.
 *
 * @param fat16_ins 文件系统指针
 * @param Dir       文件的目录项
 * @param count     输出参数, 当为 NULL 时忽略该参数, 否则设置为文件当前簇的数量
 * @return WORD     文件最后一个簇的编号
 */
WORD file_last_cluster(FAT16 *fat16_ins, DIR_ENTRY *Dir, int64_t *count)
{
    int64_t cnt = 0;            // 文件拥有的簇数量
    WORD    cur = CLUSTER_END;  // 最后一个被文件使用的簇号
    // TODO: 找到Dir对应的文件的最后一个簇编号, 并将该文件当前簇的数目填充入count
    // HINT: 可能用到的函数: is_cluster_inuse和fat_entry_by_cluster函数.
    /*** BEGIN ***/
    WORD clusterN        = Dir->DIR_FstClusLO;
    WORD fatClusEntryVal = Dir->DIR_FstClusLO;
    if (is_cluster_inuse(clusterN)) {
        do {
            cnt++;
            clusterN        = fatClusEntryVal;
            fatClusEntryVal = fat_entry_by_cluster(fat16_ins, clusterN);
        } while (is_cluster_inuse(fatClusEntryVal));
        cur = clusterN;
    }
    /*** END ***/
    if (count != NULL) {  // 如果count为NULL, 不填充count
        *count = cnt;
    }
    return cur;
}

/**
 * @brief 为 Dir 指向的文件新分配 count 个簇, 并将其连接在文件末尾,
 *        保证新分配的簇全部以 0 填充. 注意, 如果文件当前没有任何簇, 本函数应该修改
 *        Dir->DIR_FstClusLO 值, 使其指向第一个簇.
 *
 * @param fat16_ins     文件系统指针
 * @param Dir           要分配新簇的文件的目录项
 * @param last_cluster  file_last_cluster 的返回值, 当前该文件的最后一个簇簇号.
 * @param count         要新分配的簇数量
 * @return int 成功返回分配前原文件最后一个簇簇号, 失败返回 POSIX 错误代码的负值
 */
int file_new_cluster(FAT16 *fat16_ins, DIR_ENTRY *Dir, WORD last_cluster,
                     DWORD count)
{
    /** TODO: 先用 alloc_clusters 分配 count 个簇. 然后若原文件本身有至少一个簇,
     *        修改原文件最后一个簇的 FAT 表项, 使其与新分配的簇连接. 否则修改
     *        Dir->DIR_FstClusLO 值, 使其指向第一个簇.
     */
    /*** BEGIN ***/
    if (count == 0) {
        return last_cluster;
    }
    if (last_cluster == CLUSTER_END) {
        Dir->DIR_FstClusLO = alloc_clusters(fat16_ins, count);
        if (Dir->DIR_FstClusLO == CLUSTER_END) {
            return -ENOSPC;
        }
    } else {
        WORD newCluster = alloc_clusters(fat16_ins, count);
        if (newCluster == CLUSTER_END) {
            return -ENOSPC;
        }
        write_fat_entry(fat16_ins, last_cluster, newCluster);
    }
    /*** END ***/
    return last_cluster;
}

/**
 * @brief 在文件 offset 的位置写入 buff 中的数据, 数据长度为 length.
 *
 * @param fat16_ins   文件系统执政
 * @param Dir         要写入的文件目录项
 * @param offset_dir  find_root 返回的 offset_dir 值
 * @param buff        要写入的数据
 * @param offset      文件要写入的位置
 * @param length      要写入的数据长度 (字节)
 * @return int        成功时返回成功写入数据的字节数, 失败时返回 POSIX 错误代码的负值
 */
int write_file(FAT16 *fat16_ins, DIR_ENTRY *Dir, off_t offset_dir, const void *buff,
               off_t offset, size_t length)
{
    if (length == 0)
        return 0;

    if (offset + length < offset)  // 溢出了
        return -EINVAL;

    /** TODO: 通过 offset 和 length, 判断文件是否修改文件大小, 以及是否需要分配新簇,
     *        并正确修改大小和分配簇.
     * HINT: 可能用到的函数: file_last_cluster, file_new_cluster 等
     */
    /*** BEGIN ***/
    size_t newSize = offset + length;
    if (newSize > Dir->DIR_FileSize) {
        Dir->DIR_FileSize  = newSize;
        int64_t newClusNum = ((newSize - 1) / fat16_ins->ClusterSize) + 1;
        int64_t curClusNum;
        WORD    lastClus = file_last_cluster(fat16_ins, Dir, &curClusNum);

        if (newClusNum > curClusNum) {
            int ret =
                file_new_cluster(fat16_ins, Dir, lastClus, newClusNum - curClusNum);
            if (ret < 0) {
                return ret;
            }
        }
        dir_entry_write(fat16_ins, offset_dir, Dir);
    }
    /*** END ***/

    /** TODO: 和 read 类似, 找到对应的偏移, 并写入数据.
     *  HINT: 如果你正确实现了 write_to_cluster_at_offset, 此处逻辑会简单很多.
     */
    /*** BEGIN ***/
    // HINT: 记得把修改过的Dir写回目录项 (如果你之前没有写回)
    // HINT: 记得把修改过的Dir写回目录项 (如果你之前没有写回)
    DWORD  beginBytes  = offset;
    DWORD  endBytes    = offset + length - 1;
    DWORD  beginClus   = beginBytes / fat16_ins->ClusterSize;
    DWORD  endClus     = endBytes / fat16_ins->ClusterSize;
    off_t  beginOffset = beginBytes % fat16_ins->ClusterSize;
    off_t  endOffset   = endBytes % fat16_ins->ClusterSize;
    size_t writeSize   = 0;

    // 找到 begin_cluster
    WORD clusNum = Dir->DIR_FstClusLO;
    WORD clusCnt = 0;
    while (clusCnt < beginClus) {
        clusNum = fat_entry_by_cluster(fat16_ins, clusNum);
        clusCnt++;
        if (clusNum == CLUSTER_END) {
            return -EIO;
        }
    }

    if (beginClus == endClus) {
        writeSize += write_to_cluster_at_offset(fat16_ins, clusNum, beginOffset,
                                                buff, endOffset - beginOffset + 1);
    } else {
        for (clusCnt = beginClus; clusCnt <= endClus; clusCnt++) {
            if (clusNum == CLUSTER_END) {
                return -EIO;
            }
            if (clusCnt == beginClus) {
                writeSize += write_to_cluster_at_offset(
                    fat16_ins, clusNum, beginOffset, buff + writeSize,
                    fat16_ins->ClusterSize - beginOffset);
            } else if (clusCnt == endClus) {
                writeSize += write_to_cluster_at_offset(
                    fat16_ins, clusNum, 0, buff + writeSize, endOffset + 1);
            } else {
                writeSize += write_to_cluster_at_offset(
                    fat16_ins, clusNum, 0, buff + writeSize, fat16_ins->ClusterSize);
            }
            clusNum = fat_entry_by_cluster(fat16_ins, clusNum);
        }
    }
    /*** END ***/
    return writeSize;
}

/**
 * @brief 将长度为 size 的数据 data 写入 path 对应的文件的 offset 位置.
 *        注意当写入数据量超过文件本身大小时, 需要扩展文件的大小,
 *        必要时需要分配新的簇.
 *
 * @param path    要写入的文件的路径
 * @param data    要写入的数据
 * @param size    要写入数据的长度
 * @param offset  文件中要写入数据的偏移量 (字节)
 * @param fi      本次实验可忽略该参数
 * @return int    成功返回写入的字节数, 失败返回 POSIX 错误代码的负值.
 */
int fat16_write(const char *path, const char *data, size_t size, off_t offset,
                struct fuse_file_info *fi)
{
    FAT16 *fat16_ins = get_fat16_ins_fix();
    /** TODO: 大部分工作都在 write_file 里完成了, 这里调用 find_root 获得目录项,
     *        然后调用 write_file 即可
     */
    /*** BEGIN ***/
    DIR_ENTRY dir;
    off_t     offsetDir;
    if (find_root(fat16_ins, &dir, path, &offsetDir)) {
        return -ENOENT;
    }
    return write_file(fat16_ins, &dir, offsetDir, data, offset, size);
    /*** END ***/
}

/**
 * @brief 将 path 对应的文件大小改为 size, 注意 size 可以大于小于或等于原文件大小.
 *        若 size 大于原文件大小, 需要将拓展的部分全部置为 0, 如有需要, 需要分配新簇.
 *        若 size 小于原文件大小, 将从末尾截断文件, 若有簇不再被使用,
 *        应该释放对应的簇.
 *        若 size 等于原文件大小, 什么都不需要做.
 *
 * @param path 需要更改大小的文件路径
 * @param size 新的文件大小
 * @return int 成功返回 0, 失败返回 POSIX 错误代码的负值.
 */
int fat16_truncate(const char *path, off_t size)
{
    /* Gets volume data supplied in the context during the fat16_init function */
    FAT16 *fat16_ins = get_fat16_ins_fix();

    /* Searches for the given path */
    DIR_ENTRY dir;
    off_t     offsetDir;
    find_root(fat16_ins, &dir, path, &offsetDir);

    // 当前文件已有簇的数量, 以及截断或增长后, 文件所需的簇数量.
    int64_t curClusCnt;
    WORD    lastClus   = file_last_cluster(fat16_ins, &dir, &curClusCnt);
    int64_t newClusCnt = (size - 1) / fat16_ins->ClusterSize + 1;

    DWORD newSize = size;
    DWORD oldSize = dir.DIR_FileSize;

    if (oldSize == newSize) {
        return 0;
    } else if (oldSize < newSize) {
        /** TODO: 增大文件大小, 注意是否需要分配新簇, 以及往新分配的空间填充 0 等 **/
        /*** BEGIN ***/
        if (newClusCnt > curClusCnt) {
            int ret =
                file_new_cluster(fat16_ins, &dir, lastClus, newClusCnt - curClusCnt);
            if (ret < 0) {
                return ret;
            }
        }
        off_t  oldOffset = oldSize % fat16_ins->ClusterSize;
        size_t clearSize = fat16_ins->ClusterSize - oldOffset;
        BYTE   emptyBuffer[clearSize];
        memset(emptyBuffer, 0, clearSize);
        write_to_cluster_at_offset(fat16_ins, lastClus, oldOffset, emptyBuffer,
                                   clearSize);
        /*** END ***/

    } else {  // 截断文件
        /** TODO: 截断文件, 注意是否需要释放簇等 **/
        /*** BEGIN ***/
        if (newClusCnt < curClusCnt) {
            // 找到最后一个簇
            WORD newLastClus = dir.DIR_FstClusLO;  // 最后一个簇的簇号
            WORD firstFreeClus = dir.DIR_FstClusLO;  // 被释放的第一个的簇的簇号
            WORD firstSecOfClus;
            BYTE sectorBuffer[BYTES_PER_SECTOR];
            for (int i = 0; i < curClusCnt; i++) {
                newLastClus = firstFreeClus;
                first_sector_by_cluster(fat16_ins, newLastClus, &firstFreeClus,
                                        &firstSecOfClus, sectorBuffer);
            }
            // 修改最后一个簇 FAT 表项
            write_fat_entry(fat16_ins, newLastClus, CLUSTER_END);
            // 释放后面的簇
            while(is_cluster_inuse(firstFreeClus)) {
                firstFreeClus = free_cluster(fat16_ins, firstFreeClus);
            }
        }
        /*** END ***/
    }
    // 写回文件大小
    dir.DIR_FileSize = newSize;
    dir_entry_write(fat16_ins, offsetDir, &dir);
    return 0;
}
```

# 附加部分
> 为了减少不必要的复制粘贴, 这部分对要修改的函数进行重载处理, 具体方法见 [Stack Overflow](https://stackoverflow.com/a/617588) (只对 gcc 有效). 即
> 
> 1. 给要重载的函数定义打上 `__attribute__((weak))`.
> 2. 在别处写上新的重载函数.
> 
> 具体被重载的函数见下面的介绍与 [`part_1.c`](./simple_fat16_part1.c) 和 [`part_2.c`](./simple_fat16_part2.c)
## 实现文件系统的三备份容错机制
### 具体修改
重载了 `pre_init_fat16()`, `sector_read()`, `sector_write()` 函数, 并添加了 `FILE *backup_fd[2]` 和 `const char *BACKUP_FILE_NAME[2]` 两个全局变量, 具体可见 [`backup.h`](./backup.h) 和 [`backup.c`](simple_fat16_backup.c).

1. 在 `pre_init_fat16()` 中初始化 `FILE *backup_fd[2]`
2. 在 `sector_read()` 中, 每次读取都对原始镜像文件和两个备份镜像文件的对应扇区进行读取, 然后逐字节对比三个扇区, 若发现不同则定位并修改被错误的 bit, 输出错误信息.
3. 在 `sector_write()` 中, 每次都对三个镜像文件进行写入.

如下
```c
const char *BACKUP_FILE_NAME[2] = {"fat16.img.bak", "fat16.img.bak2"};
FILE       *backup_fd[2];

void sector_read(FILE *fd, unsigned int secnum, void *buffer)
{
    BYTE origin_buffer[BYTES_PER_SECTOR];
    fseek(fd, BYTES_PER_SECTOR * secnum, SEEK_SET);
    fread(origin_buffer, BYTES_PER_SECTOR, 1, fd);

    FILE *backup_fd_copy[2] = {backup_fd[0], backup_fd[1]};
    BYTE  backup_buffer[2][BYTES_PER_SECTOR];
    fseek(backup_fd_copy[0], BYTES_PER_SECTOR * secnum, SEEK_SET);
    fread(backup_buffer[0], BYTES_PER_SECTOR, 1, backup_fd_copy[0]);
    fseek(backup_fd_copy[1], BYTES_PER_SECTOR * secnum, SEEK_SET);
    fread(backup_buffer[1], BYTES_PER_SECTOR, 1, backup_fd_copy[1]);

    int find_flag     = 0;
    int origin_bit    = 0;
    int backup_bit[2] = {0, 0};
    int correct_bit   = 0;
    for (int i = 0; i < BYTES_PER_SECTOR; i++) {
        if (origin_buffer[i] == backup_buffer[0][i]
            && origin_buffer[i] == backup_buffer[1][i])
            continue;

        find_flag = 1;
        for (int j = 0; j < 8; j++) {
            origin_bit    = (origin_buffer[i] >> j) & 0x01;
            backup_bit[0] = (backup_buffer[0][i] >> j) & 0x01;
            backup_bit[1] = (backup_buffer[1][i] >> j) & 0x01;
            if (origin_bit != backup_bit[0] || origin_bit != backup_bit[1]) {
                fprintf(stderr,
                        "\nERROR: Found uncompatible data at sector %u, byte %d, "
                        "bit %d\n",
                        secnum, i, j);
                correct_bit = (origin_bit + backup_bit[0] + backup_bit[1]) >> 1;
                fprintf(stderr,
                        "\tCorrect bit %1d, origin bit %1d, "
                        "backup bit %1d & %1d\n\n",
                        correct_bit, origin_bit, backup_bit[0], backup_bit[1]);
                if (correct_bit) {
                    origin_buffer[i]    |= (BYTE)(1 << j);
                    backup_buffer[0][i] |= (BYTE)(1 << j);
                    backup_buffer[1][i] |= (BYTE)(1 << j);
                } else {
                    origin_buffer[i]    &= ~(BYTE)(1 << j);
                    backup_buffer[0][i] &= ~(BYTE)(1 << j);
                    backup_buffer[1][i] &= ~(BYTE)(1 << j);
                }
            }
        }
    }
    if (find_flag) {
        sector_write(fd, secnum, origin_buffer);
    }
    memcpy(buffer, origin_buffer, BYTES_PER_SECTOR);
}

void sector_write(FILE *fd, unsigned int secnum, const void *buffer)
{
    fseek(fd, BYTES_PER_SECTOR * secnum, SEEK_SET);
    fwrite(buffer, BYTES_PER_SECTOR, 1, fd);
    fflush(fd);

    FILE *backup_fd_copy[2] = {backup_fd[0], backup_fd[1]};
    fseek(backup_fd_copy[0], BYTES_PER_SECTOR * secnum, SEEK_SET);
    fwrite(buffer, BYTES_PER_SECTOR, 1, backup_fd_copy[0]);
    fflush(backup_fd_copy[0]);

    fseek(backup_fd_copy[1], BYTES_PER_SECTOR * secnum, SEEK_SET);
    fwrite(buffer, BYTES_PER_SECTOR, 1, backup_fd_copy[1]);
    fflush(backup_fd_copy[1]);
}

FAT16 *pre_init_fat16(const char *imageFilePath)
{
    /* Opening the FAT16 image file */
    FILE *fd = fopen(imageFilePath, "rb+");

    if (fd == NULL) {
        fprintf(stderr, "Missing FAT16 image file!\n");
        exit(EXIT_FAILURE);
    }

    backup_fd[0] = fopen(BACKUP_FILE_NAME[0], "rb+");
    backup_fd[1] = fopen(BACKUP_FILE_NAME[1], "rb+");
    if (backup_fd[0] == NULL || backup_fd[1] == NULL) {
        fprintf(stderr, "Missing backup file!\n");
        exit(EXIT_FAILURE);
    }

    FAT16 *fat16_ins = malloc(sizeof(FAT16));

    fat16_ins->fd = fd;

    /* Reads the BPB */
    sector_read(fat16_ins->fd, 0, &fat16_ins->Bpb);

    /* First sector of the root directory */
    fat16_ins->FirstRootDirSecNum =
        fat16_ins->Bpb.BPB_RsvdSecCnt
        + (fat16_ins->Bpb.BPB_FATSz16 * fat16_ins->Bpb.BPB_NumFATS);

    /* Number of sectors in the root directory */
    DWORD RootDirSectors =
        ((fat16_ins->Bpb.BPB_RootEntCnt * 32) + (fat16_ins->Bpb.BPB_BytsPerSec - 1))
        / fat16_ins->Bpb.BPB_BytsPerSec;

    /* First sector of the data region (cluster #2) */
    fat16_ins->FirstDataSector =
        fat16_ins->Bpb.BPB_RsvdSecCnt
        + (fat16_ins->Bpb.BPB_NumFATS * fat16_ins->Bpb.BPB_FATSz16) + RootDirSectors;

    fat16_ins->FatOffset =
        fat16_ins->Bpb.BPB_RsvdSecCnt * fat16_ins->Bpb.BPB_BytsPerSec;
    fat16_ins->FatSize = fat16_ins->Bpb.BPB_BytsPerSec * fat16_ins->Bpb.BPB_FATSz16;
    fat16_ins->RootOffset =
        fat16_ins->FatOffset + fat16_ins->FatSize * fat16_ins->Bpb.BPB_NumFATS;
    fat16_ins->ClusterSize =
        fat16_ins->Bpb.BPB_BytsPerSec * fat16_ins->Bpb.BPB_SecPerClus;
    fat16_ins->DataOffset =
        fat16_ins->RootOffset + fat16_ins->Bpb.BPB_RootEntCnt * BYTES_PER_DIR;

    return fat16_ins;
}
```

同时还需要修改 `Makefile`, 添加下面的项
```makefile
backup: simple_fat16_part1.o simple_fat16_part2.o simple_fat16_backup.o
	$(CC) $(CFLAGS) -o simple_fat16 $^ $(LDFLAGS) $(LDLIBS)
simple_fat16_backup.o: simple_fat16_backup.c fat16.h backup.h
	$(CC) $(CFLAGS) -c -o $@ $<
```

### 测试
为了反转镜像文件的某一个 bit, 可以在挂载前修改镜像文件, 为此, 编写了下面的小程序, 可以修改特定镜像文件对应扇区对应字节的某一 bit. 用法如下 (注意参数全部从 0 开始计算)
```
Usage: 
  bit_flip --help
    Show this help message.
  bit_flip
    Filp a random bit of random img.
  bit_flip [img #] [sector #] [byte #] [bit #]
    Filp the bit # of the sector #, byte # of img #.
      img #: 0 for origin img, 1 for backup img 1 and 2 for backup img 2.
```
程序代码如下
```c
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define IMG_SIZE         31457280
#define BYTES_PER_SECTOR 512
#define SECTOR_NUM       (IMG_SIZE / BYTES_PER_SECTOR)

const char *filename[3] = {"fat16.img", "fat16.img.bak", "fat16.img.bak2"};

int main(int argc, char *argv[])
{
    long img_num  = 0;
    long sec_num  = 0;
    long byte_num = 0;
    long bit_num  = 0;

    if (argc == 1) {
        printf("A random bit of random img will be flipped. Continue? (y/n) ");
        char c = getchar();
        if (c != 'y' && c != 'Y') {
            return 0;
        }
        srand(time(NULL));
        img_num  = rand() % 3;
        sec_num  = rand() % SECTOR_NUM;
        byte_num = rand() % BYTES_PER_SECTOR;
        bit_num  = rand() % 8;
    } else if (strcmp(argv[1], "--help") == 0) {
        printf("Usage: \n");
        printf("  bit_flip --help\n");
        printf("    Show this help message.\n");
        printf("  bit_flip\n");
        printf("    Filp a random bit of random img.\n");
        printf("  bit_flip [img #] [sector #] [byte #] [bit #]\n");
        printf("    Filp the bit # of the sector #, byte # of img #.\n");
        printf("      img #: 0 for origin img, 1 for backup img 1 and 2 for backup "
               "img 2.\n");
        return 0;
    } else if (argc == 5) {
        img_num  = strtol(argv[1], NULL, 10);
        sec_num  = strtol(argv[2], NULL, 10);
        byte_num = strtol(argv[3], NULL, 10);
        bit_num  = strtol(argv[4], NULL, 10);
    } else {
        printf("Invalid arguments.\n");
        return 0;
    }

    if (img_num < 0 || img_num > 2 || sec_num < 0 || sec_num >= SECTOR_NUM
        || byte_num < 0 || byte_num >= BYTES_PER_SECTOR || bit_num < 0
        || bit_num >= 8)
    {
        printf("Invalid arguments.\n");
        return 0;
    }

    printf("Flipping bit %ld of sector %ld, byte %ld of img %s.\n", bit_num, sec_num,
           byte_num, filename[img_num]);

    FILE *img_fd = fopen(filename[img_num], "rb+");
    long  offset = sec_num * BYTES_PER_SECTOR + byte_num;

    fseek(img_fd, offset, SEEK_SET);
    uint8_t byte = fgetc(img_fd);
    byte         ^= (1 << bit_num);
    fseek(img_fd, offset, SEEK_SET);
    fputc(byte, img_fd);
    fclose(img_fd);
}
```
同时编写了对应的脚本实现自动测试, 测试流程如下

1. 拷贝两份镜像文件备份
2. 检查镜像文件和备份的 MD5 校验码
3. 使用 `bit_filp` 反转 bit
4. 检查反转后的镜像文件和备份的 MD5 校验码
5. 使用没有三备份容错机制的文件系统挂载镜像, 可以查看被修改的部分
6. 使用三备份容错机制的文件系统挂载镜像, 重新查看被修改的部分, 注意观察输出
7. 重新检查镜像文件和备份的 MD5 校验码, 结束测试

脚本如下

```sh
#!/usr/bin/bash
gcc -O2 -o bit_flip bit_flip.c

echo ""
echo "Initializing backup file..."
cp fat16.img fat16.img.bak
cp fat16.img fat16.img.bak2
echo ""

echo "Origin MD5 sum:"
md5sum fat16.img
md5sum fat16.img.bak
md5sum fat16.img.bak2
echo ""

echo "Calling ./bit_flip"
echo ""
if [[ $# -eq 0 ]]; then
    ./bit_flip
elif [[ $# -eq 1 ]]; then
    ./bit_flip $1
    exit 0
elif [[ $# -eq 4 ]]; then
    ./bit_flip $1 $2 $3 $4
else
    echo "Invalid arguments"
    exit 1
fi
echo ""

echo ""
echo "New MD5 sum:"
md5sum fat16.img
md5sum fat16.img.bak
md5sum fat16.img.bak2
echo ""

echo ""
while true; do
    read -p "Continue Mounting Image WITHOUT Backup Check? (Y/n)" yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer y(es) or n(o).";;
    esac
done

make default;
./simple_fat16 -s -d ./fat_dir
echo ""

while true; do
    read -p "Continue Mounting Image WITH Backup Check? (Y/n)" yn
    case $yn in
        [Yy]* ) break;;
        [Nn]* ) exit;;
        * ) echo "Please answer y(es) or n(o).";;
    esac
done

make backup;
./simple_fat16 -s -d ./fat_dir

echo ""

echo "Recheck MD5 sum:"
md5sum fat16.img
md5sum fat16.img.bak
md5sum fat16.img.bak2
```

比如第 161 个扇区的第 13 个字节的第 6 位属于 `/log.c` 的第 1 行的第 13 个字符 `r`, 反转后变为 `R`. (全部从 1 开始计数) 对该 bit 进行测试, 输出如下
```plaintext
$ ./backup.sh 0 160 12 5        

Initializing backup file...

Origin MD5 sum:
5e4f28aa93856f5b8aadb76c952f31d0  fat16.img
5e4f28aa93856f5b8aadb76c952f31d0  fat16.img.bak
5e4f28aa93856f5b8aadb76c952f31d0  fat16.img.bak2

Calling ./bit_flip

Flipping bit 5 of sector 160, byte 12 of img fat16.img.


New MD5 sum:
fa41da0c5fd9058e230a612eaf9d8e08  fat16.img
5e4f28aa93856f5b8aadb76c952f31d0  fat16.img.bak
5e4f28aa93856f5b8aadb76c952f31d0  fat16.img.bak2


Continue Mounting Image WITHOUT Backup Check? (Y/n)Y
...

Continue Mounting Image WITH Backup Check? (Y/n)Y
...

ERROR: Found uncompatible data at sector 160, byte 12, bit 5
        Correct bit 1, origin bit 0, backup bit 1 & 1

...

Recheck MD5 sum:
5e4f28aa93856f5b8aadb76c952f31d0  fat16.img
5e4f28aa93856f5b8aadb76c952f31d0  fat16.img.bak
5e4f28aa93856f5b8aadb76c952f31d0  fat16.img.bak2
```

其中在另一个终端中的输入输出如下
```plaintext
$ cat fat_dir/log.c
#include <erRno.h>  # 2nd 'r' -> `R`
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#define LOGNAME "mount_fat16.log"


static FILE *logfile;

void log_open()
{
  logfile = fopen(LOGNAME, "w");

  if (logfile == NULL) {
    perror("[ERRO] Could not open log file.");
    exit(EXIT_FAILURE);
  }

  // Set logfile to line buffering
  setvbuf(logfile, NULL, _IONBF, 0);
}

void log_msg(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(logfile, format, ap);
}
$ fusermount -uz ./fat_dir
$ cat fat_dir/log.c
#include <errno.h>  # recovered
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

#define LOGNAME "mount_fat16.log"


static FILE *logfile;

void log_open()
{
  logfile = fopen(LOGNAME, "w");

  if (logfile == NULL) {
    perror("[ERRO] Could not open log file.");
    exit(EXIT_FAILURE);
  }

  // Set logfile to line buffering
  setvbuf(logfile, NULL, _IONBF, 0);
}

void log_msg(const char *format, ...)
{
  va_list ap;
  va_start(ap, format);
  vfprintf(logfile, format, ap);
}
$ fusermount -uz ./fat_dir
```

## 实现文件系统的日志机制
具体的实现可见 [`log.h`](./log.h) 和 [`log.c`](./simple_fat16_log.c).

日志系统的主要难点在于要求线程安全, 需要使用课堂上讲的 Reader-Writer Problem 的解决方案对镜像文件加锁, 并对每一个文件操作都加锁, 需要添加的函数有

- `void log_open();`
- `void log_printf(const char *format, ...);`
- `void down(mutex *m);`
- `void up(mutex *m);`

需要重载的函数有:  

- `pre_init_fat16()`
- `sector_read()`
- `sector_write()`
- `fat16_readdir()`
- `fat16_read()`
- `fat16_mknod()`
- `fat16_unlink()`
- `fat16_mkdir()`
- `fat16_rmdir()`
- `fat16_write()`
- `fat16_truncate()`

需要添加的全局变量有 

- `const char *log_filename`
- `FILE *log_fd`
- `mutex file_operations_mutex`
- `mutex file_read_write_mutex`
- `mutex reader_count_mutex`
- `mutex log_mutex`
- `int reader_count`

其中对文件操作的修改主要是在函数开头占用 `file_operations_mutex` 并对日志输出 `[BEGIN] ...`, 在返回时输出可能的错误信息和 `[END] ...`, 并释放 `file_operations_mutex`, 后面就不再阐述这一部分.

首先是日志操作
```c
void log_open()
{
    log_fd = fopen(log_filename, "a");

    if (log_fd == NULL) {
        perror("[ERRO] Could not open log file.");
        exit(EXIT_FAILURE);
    }

    // Set log_fd to line buffering
    setvbuf(log_fd, NULL, _IONBF, 0);
}

void log_printf(const char *format, ...)
{
    down(&log_mutex);

    va_list args;
    va_start(args, format);
    vfprintf(log_fd, format, args);
    va_end(args);
    fflush(log_fd);

    up(&log_mutex);
}
```

然后是对锁的 PV 操作, 如下
```c
void down(mutex *m)
{
    while (*m == 0)
        ;
    *m = 0;
}

void up(mutex *m)
{
    *m = 1;
}
```

最后是初始化和扇区读写, 具体的线程安全解决方案可以搜索 Reader-Writer Problem.
```c
FAT16 *pre_init_fat16(const char *imageFilePath)
{
    down(&file_operations_mutex);
    /* open log file */
    log_open();
    log_printf("[BEGIN] Mounting FAT16 image file %s.\n", imageFilePath);

    /* Opening the FAT16 image file */
    FILE *fd = fopen(imageFilePath, "rb+");

    if (fd == NULL) {
        fprintf(stderr, "Missing FAT16 image file!\n");
        log_printf("[ERROR] Missing FAT16 image file.\n");
        up(&file_operations_mutex);
        exit(EXIT_FAILURE);
    }

    FAT16 *fat16_ins = malloc(sizeof(FAT16));

    fat16_ins->fd = fd;

    /* Reads the BPB */
    sector_read(fat16_ins->fd, 0, &fat16_ins->Bpb);

    /* First sector of the root directory */
    fat16_ins->FirstRootDirSecNum =
        fat16_ins->Bpb.BPB_RsvdSecCnt
        + (fat16_ins->Bpb.BPB_FATSz16 * fat16_ins->Bpb.BPB_NumFATS);

    /* Number of sectors in the root directory */
    DWORD RootDirSectors =
        ((fat16_ins->Bpb.BPB_RootEntCnt * 32) + (fat16_ins->Bpb.BPB_BytsPerSec - 1))
        / fat16_ins->Bpb.BPB_BytsPerSec;

    /* First sector of the data region (cluster #2) */
    fat16_ins->FirstDataSector =
        fat16_ins->Bpb.BPB_RsvdSecCnt
        + (fat16_ins->Bpb.BPB_NumFATS * fat16_ins->Bpb.BPB_FATSz16) + RootDirSectors;

    fat16_ins->FatOffset =
        fat16_ins->Bpb.BPB_RsvdSecCnt * fat16_ins->Bpb.BPB_BytsPerSec;
    fat16_ins->FatSize = fat16_ins->Bpb.BPB_BytsPerSec * fat16_ins->Bpb.BPB_FATSz16;
    fat16_ins->RootOffset =
        fat16_ins->FatOffset + fat16_ins->FatSize * fat16_ins->Bpb.BPB_NumFATS;
    fat16_ins->ClusterSize =
        fat16_ins->Bpb.BPB_BytsPerSec * fat16_ins->Bpb.BPB_SecPerClus;
    fat16_ins->DataOffset =
        fat16_ins->RootOffset + fat16_ins->Bpb.BPB_RootEntCnt * BYTES_PER_DIR;

    log_printf("[END] Mounting FAT16 image file %s succeeded.\n", imageFilePath);
    up(&file_operations_mutex);
    return fat16_ins;
}

void sector_read(FILE *fd, unsigned int secnum, void *buffer)
{
    down(&reader_count_mutex);
    if (++reader_count == 1)
        down(&file_read_write_mutex);
    up(&reader_count_mutex);

    fseek(fd, BYTES_PER_SECTOR * secnum, SEEK_SET);
    fread(buffer, BYTES_PER_SECTOR, 1, fd);

    down(&reader_count_mutex);
    if (--reader_count == 0)
        up(&file_read_write_mutex);
    up(&reader_count_mutex);
}

void sector_write(FILE *fd, unsigned int secnum, const void *buffer)
{
    down(&file_read_write_mutex);

    fseek(fd, BYTES_PER_SECTOR * secnum, SEEK_SET);
    fwrite(buffer, BYTES_PER_SECTOR, 1, fd);
    fflush(fd);

    up(&file_read_write_mutex);
}
```

以及对 Makefile 的修改
```makefile
log: simple_fat16_part1.o simple_fat16_part2.o simple_fat16_log.o
	$(CC) $(CFLAGS) -o simple_fat16 $^ $(LDFLAGS) $(LDLIBS)
simple_fat16_log.o: simple_fat16_log.c fat16.h log.h
	$(CC) $(CFLAGS) -c -o $@ $<
```


## 二者结合
结合起来就是 [simple_fat_optional.c](./simple_fat16_optional.c), 不再赘述. 使用 `make all` 即可生成. Makefile 如下
```makefile
all: simple_fat16_all

simple_fat16_all: simple_fat16_part1.o simple_fat16_part2.o simple_fat16_optional.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

simple_fat16_optional.o: simple_fat16_optional.c fat16.h backup.h log.h
	$(CC) $(CFLAGS) -c -o $@ $<
```