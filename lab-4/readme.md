> 代码使用 clang-format 格式化, ColumnLimit 为 85, 你可能需要使用宽屏获得较好的代码阅读体验
# Tips
- 在 Part 1 开始之前可以看看 `find_root()` 和 `find_subdir()` 函数, 其遍历方法就是需要填入的代码的遍历方法, `find_root()` 函数也是之后需要调用的函数.
- 目录项 `0x00` 偏移 (即 `DIR_Name[0]` 位置) 为 `0x00` 时表示空闲项, `0xe5` 表示已删除项, `0x0f` 表示长文件名项.
- `find_root()` 返回 0 时表示找到
- `DIR_ENTRY` 结构体的 `DIR_Name` 元素并不是字符串, 因为其没有以 `\0` 结尾, 所以与 `paths[-1]` 比较时应该使用 `strncmp()` 函数, 返回 0 是表示两字符串相等.

# Part 1 (Complete in 6 Hour)

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
                    if (FatClusEntryVal == 0xffff) {
                        return 0;
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
            return read_size;
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
                    if (FatClusEntryVal == 0xffff) {
                        return 0;
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
| `DIR_CrtTimeTenth` | `0x0D` |   1    |              保留 (FAT32 中用作创建时间，精确到 10ms)               |
|   `DIR_CrtTime`    | `0x0E` |   2    |               保留 (FAT32 中用作创建时间，精确到 2s)                |
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
            } else if (!is_deleted || !is_lfn) {
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
            } else if (!is_deleted || !is_lfn) {
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
                    if (FatClusEntryVal == 0xffff) {
                        return 0;
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