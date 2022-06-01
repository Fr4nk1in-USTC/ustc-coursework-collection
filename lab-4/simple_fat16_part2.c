#include "fat16.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

// 忘记写到fat16.h 里的函数定义
extern FAT16 *get_fat16_ins();

/**
 * @brief 请勿修改该函数.
 * 该函数用于修复 5 月 13 日发布的 simple_fat16_part1.c 中 RootOffset 和 DataOffset
 * 的计算错误. 如果你在 Part1 中也使用了以下字段或函数:
 *     fat16_ins->RootOffset
 *     fat16_ins->DataOffset
 *     find_root 函数的 offset_dir 输出参数
 * 请手动修改 pre_init_fat16 函数定义中 fat16_ins->RootOffset 的计算, 如下:
 * 正确的计算:
 *      fat16_ins->RootOffset = fat16_ins->FatOffset + fat16_ins->FatSize *
 *                              fat16_ins->Bpb.BPB_NumFATS;
 * 错误的计算 (5月13日发布的simple_fat16_part1.c中的版本) :
 *   // fat16_ins->RootOffset = fat16_ins->FatOffset * fat16_ins->FatSize *
 *   //                         fat16_ins->Bpb.BPB_NumFATS;
 * 即将 RootOffset 计算中第一个乘号改为加号.
 * @return FAT16* 修复计算并返回文件系统指针
 */
FAT16 *get_fat16_ins_fix()
{
    FAT16 *fat16_ins = get_fat16_ins();
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

/**
 * @brief 簇号是否是合法的 (表示正在使用的) 数据簇号 (在 CLUSTER_MIN 和 CLUSTER_MAX
 *        之间)
 *
 * @param cluster_num 簇号
 * @return int
 */
int is_cluster_inuse(uint16_t cluster_num)
{
    return CLUSTER_MIN <= cluster_num && cluster_num <= CLUSTER_MAX;
}

/**
 * @brief 将 data 写入簇号为 clusterN 的簇对应的 FAT 表项, 注意要对文件系统中所有 FAT
 *        表都进行相同的写入.
 *
 * @param fat16_ins 文件系统指针
 * @param clusterN  要写入表项的簇号
 * @param data      要写入表项的数据, 如下一个簇号, CLUSTER_END (文件末尾) , 或者0
 *                  (释放该簇) 等等
 * @return int      成功返回 0
 */
int write_fat_entry(FAT16 *fat16_ins, WORD clusterN, WORD data)
{
    // Hint: 这个函数逻辑与 fat_entry_by_cluster 函数类似,
    // 但这个函数需要修改对应值并写回 FAT 表中
    BYTE SectorBuffer[BYTES_PER_SECTOR];
    /** TODO: 计算下列值, 当然, 你也可以不使用这些变量*/
    uint firstFatSecNum;  // 第一个 FAT 表开始的扇区号
    uint clusterOffset;  // clusterN 这个簇对应的表项, 在每个 FAT 表项的哪个偏移量
    uint clusterSec;  // clusterN 这个簇对应的表项, 在每个 FAT 表中的第几个扇区
                      // (Hint: 这个值与 clusterSec 的关系是?)
    uint secOffset;  // clusterN 这个簇对应的表项, 在所在扇区的哪个偏移量
                     // (Hint: 这个值与 clusterSec 的关系是?)
    /*** BEGIN ***/
    firstFatSecNum = fat16_ins->Bpb.BPB_RsvdSecCnt;
    clusterOffset  = clusterN * 2;
    clusterSec     = clusterOffset / fat16_ins->Bpb.BPB_BytsPerSec;
    secOffset      = clusterOffset % fat16_ins->Bpb.BPB_BytsPerSec;
    /*** END ***/
    // Hint: 对系统中每个 FAT 表都进行写入
    for (uint i = 0; i < fat16_ins->Bpb.BPB_NumFATS; i++) {
        /*** BEGIN ***/
        // Hint: 计算出当前要写入的 FAT 表扇区号
        // Hint: 读扇区, 在正确偏移量将值修改为 data, 写回扇区
        uint secNum = i * fat16_ins->Bpb.BPB_FATSz16 + firstFatSecNum + clusterSec;
        sector_read(fat16_ins->fd, secNum, SectorBuffer);
        memcpy(SectorBuffer + secOffset, &data, sizeof(WORD));
        sector_write(fat16_ins->fd, secNum, SectorBuffer);
        /*** END ***/
    }
    return 0;
}

/**
 * @brief 分配 n 个空闲簇, 分配过程中将 n 个簇通过 FAT 表项连在一起,
 *        然后返回第一个簇的簇号. 最后一个簇的 FAT 表项将会指向 0xFFFF (即文件中止).
 * @param fat16_ins 文件系统指针
 * @param n         要分配簇的个数
 * @return WORD     分配的第一个簇, 分配失败, 将返回 CLUSTER_END, 若 n==0, 也将返回
 *                  CLUSTER_END.
 */
WORD alloc_clusters(FAT16 *fat16_ins, uint32_t n)
{
    if (n == 0)
        return CLUSTER_END;

    // Hint: 用于保存找到的n个空闲簇, 另外在末尾加上CLUSTER_END, 共n+1个簇号
    WORD *clusters  = malloc((n + 1) * sizeof(WORD));
    uint  allocated = 0;  // 已找到的空闲簇个数

    /** TODO:
     * 扫描 FAT 表, 找到 n 个空闲的簇, 存入 cluster 数组.
     * 注意此时不需要修改对应的 FAT 表项
     **/
    /*** BEGIN ***/
    BYTE sectorBuffer[BYTES_PER_SECTOR];
    uint firstFatSecNum = fat16_ins->Bpb.BPB_RsvdSecCnt;
    WORD clusEntry;
    for (uint32_t i = 0; i < fat16_ins->Bpb.BPB_FATSz16; i++) {
        sector_read(fat16_ins->fd, firstFatSecNum + i, sectorBuffer);
        for (uint32_t j = 0; j < fat16_ins->Bpb.BPB_BytsPerSec; j += 2) {
            memcpy(&clusEntry, &sectorBuffer[j], sizeof(WORD));
            if (clusEntry == 0x0000) {
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

    // Hint: 找到了 n 个空闲簇, 将 CLUSTER_END 加至末尾.
    clusters[n] = CLUSTER_END;

    /** TODO: 修改 clusters 中存储的 N 个簇对应的 FAT 表项,
     *        将每个簇与下一个簇连接在一起. 同时清零每一个新分配的簇.
     **/
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

// ------------------TASK3: 创建/删除文件夹-----------------------------------

/**
 * @brief 创建 path 对应的文件夹
 *
 * @param path 创建的文件夹路径
 * @param mode 文件模式, 本次实验可忽略, 默认都为普通文件夹
 * @return int 成功:0, 失败: POSIX 错误代码的负值
 */
int fat16_mkdir(const char *path, mode_t mode)
{
    /* Gets volume data supplied in the context during the fat16_init function */
    FAT16 *fat16_ins = get_fat16_ins_fix();

    int findFlag  = 0;  // 是否找到空闲的目录项
    int sectorNum = 0;  // 找到的空闲目录项所在扇区号
    int offset    = 0;  // 找到的空闲目录项在扇区中的偏移量
    /** TODO: 模仿 mknod, 计算出 findFlag, sectorNum 和 offset 的值
     *  你也可以选择不使用这些值, 自己定义其它变量.
     *  注意本函数前半段和 mknod 前半段十分类似.
     **/
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
                        return 0;

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

    /** TODO: 在父目录的目录项中添加新建的目录. 同时, 为新目录分配一个簇,
     *        并在这个簇中创建两个目录项, 分别指向. 和 .., 目录的文件大小设置为 0
     *        即可.
     * HINT: 使用正确参数调用 dir_entry_create 来创建上述三个目录项.
     **/
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

/**
 * @brief 删除 offset 位置的目录项
 *
 * @param fat16_ins 文件系统指针
 * @param offset    find_root 传回的 offset_dir 值
 */
void dir_entry_delete(FAT16 *fat16_ins, off_t offset)
{
    BYTE buffer[BYTES_PER_SECTOR];
    /** TODO: 删除目录项, 或者说, 将镜像文件 offset 处的目录项第一个字节设置为 0xe5
     *        即可.
     * HINT: offset 对应的扇区号和扇区的偏移量是? 只需要读取扇区, 修改 offset
     *       处的一个字节, 然后将扇区写回即可.
     */
    /*** BEGIN ***/
    sector_read(fat16_ins->fd, offset / fat16_ins->Bpb.BPB_BytsPerSec, buffer);
    buffer[offset % fat16_ins->Bpb.BPB_BytsPerSec] = 0xe5;
    sector_write(fat16_ins->fd, offset / fat16_ins->Bpb.BPB_BytsPerSec, buffer);
    /*** END ***/
}

/**
 * @brief 写入 offset 位置的目录项
 *
 * @param fat16_ins 文件系统指针
 * @param offset    find_root 传回的 offset_dir 值
 * @param Dir       要写入的目录项
 */
void dir_entry_write(FAT16 *fat16_ins, off_t offset, const DIR_ENTRY *Dir)
{
    BYTE buffer[BYTES_PER_SECTOR];
    // TODO: 修改目录项, 和 dir_entry_delete 完全类似, 只是需要将整个 Dir 写入 offset
    //       所在的位置.
    /*** BEGIN ***/
    sector_read(fat16_ins->fd, offset / fat16_ins->Bpb.BPB_BytsPerSec, buffer);
    memcpy(buffer + (offset % fat16_ins->Bpb.BPB_BytsPerSec), Dir,
           sizeof(DIR_ENTRY));
    sector_write(fat16_ins->fd, offset / fat16_ins->Bpb.BPB_BytsPerSec, buffer);
    /*** END ***/
}

/**
 * @brief 删除 path 对应的文件夹
 *
 * @param path 要删除的文件夹路径
 * @return int 成功:0,  失败: POSIX错误代码的负值
 */
int fat16_rmdir(const char *path)
{
    /* Gets volume data supplied in the context during the fat16_init function */
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

    /** TODO: 检查目录是否为空, 如果目录不为空, 直接返回 -ENOTEMPTY.
     *        注意空目录也可能有 "." 和 ".." 两个子目录.
     *  HINT: 这一段和 readdir 的非根目录部分十分类似.
     *  HINT: 注意忽略 DIR_Attr 为 0x0F 的长文件名项 (LFN).
     **/
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
        if (!isDeleted || !isLFN) {
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
                    return 0;

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
    // TODO: 循环调用 free_cluster 释放对应簇, 和 unlink 类似.
    /*** BEGIN ***/
    WORD iterClusNum = dir.DIR_FstClusLO;
    while (iterClusNum != CLUSTER_END)
        iterClusNum = free_cluster(fat16_ins, iterClusNum);
    /*** END ***/

    // TODO: 删除父目录中的目录项
    // HINT: 如果你正确实现了 dir_entry_delete, 这里只需要一行代码调用它即可
    //       你也可以使用你在 unlink 使用的方法.
    /*** BEGIN ***/
    dir_entry_delete(fat16_ins, offset);
    /*** END ***/

    return 0;
}

// ------------------TASK4: 写文件-----------------------------------

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
    BYTE sector_buffer[BYTES_PER_SECTOR];
    /** TODO: 将数据写入簇对应的偏移量上. 你需要找到第一个需要写入的扇区,
     *        和要写入的偏移量, 然后依次写入后续每个扇区, 直到所有数据都写入完成.
     * 注意, offset 对应的首个扇区和 offset+size 对应的最后一个扇区
     * 都可能只需要写入一部分. 所以应该先将扇区读出, 修改要写入的部分,
     * 再写回整个扇区.
     */
    /*** BEGIN ***/

    /*** END ***/
    return size;
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

    /*** END ***/

    /** TODO: 和 read 类似, 找到对应的偏移, 并写入数据.
     *  HINT: 如果你正确实现了 write_to_cluster_at_offset, 此处逻辑会简单很多.
     */
    /*** BEGIN ***/
    // HINT: 记得把修改过的Dir写回目录项 (如果你之前没有写回)

    /*** END ***/
    return 0;
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

    /*** END ***/
    return 0;
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
    DIR_ENTRY Dir;
    off_t     offset_dir;
    find_root(fat16_ins, &Dir, path, &offset_dir);

    // 当前文件已有簇的数量, 以及截断或增长后, 文件所需的簇数量.
    int64_t cur_cluster_count;
    WORD    last_cluster = file_last_cluster(fat16_ins, &Dir, &cur_cluster_count);
    int64_t new_cluster_count =
        (size + fat16_ins->ClusterSize - 1) / fat16_ins->ClusterSize;

    DWORD new_size = size;
    DWORD old_size = Dir.DIR_FileSize;

    if (old_size == new_size) {
        return 0;
    } else if (old_size < new_size) {
        /** TODO: 增大文件大小, 注意是否需要分配新簇, 以及往新分配的空间填充 0 等 **/
        /*** BEGIN ***/

        /*** END ***/

    } else {  // 截断文件
        /** TODO: 截断文件, 注意是否需要释放簇等 **/
        /*** BEGIN ***/

        /*** END ***/
    }
    return 0;
}
