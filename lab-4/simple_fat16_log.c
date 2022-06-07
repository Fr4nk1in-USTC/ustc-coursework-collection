#include "fat16.h"
#include "log.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

const char *log_filename = "mount_fat16.log";
FILE       *log_fd;

mutex file_operations_mutex = 1;
mutex file_read_write_mutex = 1;
mutex reader_count_mutex    = 1;
mutex log_mutex             = 1;

int reader_count = 0;

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

/**
 * @brief 读取 path 对应的目录, 结果通过 filler 函数写入 buffer 中
 *
 * @param path    要读取目录的路径
 * @param buffer  结果缓冲区
 * @param filler  用于填充结果的函数, 本次实验按 filler(buffer, 文件名, NULL,
 *                0) 的方式调用即可.
 *                你也可以参考 <fuse.h> 第 58 行附近的函数声明和注释来获得更多信息.
 * @param offset  忽略
 * @param fi      忽略
 * @return int    成功返回 0, 失败返回 POSIX 错误代码的负值
 */
int fat16_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi)
{
    down(&file_operations_mutex);
    log_printf("[BEGIN] Readdir called for path %s.\n", path);

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

        if (find_root(fat16_ins, &Dir, path, &offset_dir)) {
            log_printf("[ERROR] Path %s not found.\n", path);
            log_printf("[END] Readdir failed for %s.\n", path);
            up(&file_operations_mutex);
            return -ENOENT;
        }
        if (Dir.DIR_Attr == ATTR_ARCHIVE) {
            log_printf("[ERROR] Path %s is a file.\n", path);
            log_printf("[END] Readdir failed for path %s.\n", path);
            return -ENOTDIR;
        }

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
                } else {
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

    log_printf("[END] Readdir succeeded for path %s.\n", path);
    up(&file_operations_mutex);
    return 0;
}

/**
 * @brief 从 path 对应的文件的 offset 字节处开始读取 size 字节的数据到 buffer 中,
 *        并返回实际读取的字节数.
 * Hint: 文件大小属性是 Dir.DIR_FileSize.
 *
 * @param path    要读取文件的路径
 * @param buffer  结果缓冲区
 * @param size    需要读取的数据长度
 * @param offset  要读取的数据所在偏移量
 * @param fi      忽略
 * @return int    成功返回实际读写的字符数, 失败返回 0.
 */
int fat16_read(const char *path, char *buffer, size_t size, off_t offset,
               struct fuse_file_info *fi)
{
    down(&file_operations_mutex);
    log_printf("[BEGIN] Read called for path %s, size %lu, offset %ld.\n", path,
               size, offset);

    FAT16 *fat16_ins = get_fat16_ins();

    /*** BEGIN ***/
    BYTE      sector_buffer[BYTES_PER_SECTOR];
    DIR_ENTRY Dir;
    off_t     offset_dir;
    WORD      cluster_num;         // 当前读取的簇号
    WORD      fat_clus_entry_val;  // 该簇的 FAT 表项 (下一个簇号)
    WORD      first_sec_of_clus;   // 该簇的第一个扇区号

    if (find_root(fat16_ins, &Dir, path, &offset_dir)) {
        log_printf("[ERROR] Path %s not found.\n", path);
        log_printf("[END] Read failed for %s.\n", path);
        up(&file_operations_mutex);
        return 0;
    }

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
        if (fat_clus_entry_val == CLUSTER_END) {
            log_printf("[ERROR] Cluster %d not found.\n", cluster_cnt);
            log_printf("[END] Read failed for path %s.\n", path);
            up(&file_operations_mutex);
            return 0;
        }
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
        if (fat_clus_entry_val == CLUSTER_END) {
            log_printf("[END] Read  for path %s, %lu bytes read.\n", path,
                       read_size);
            up(&file_operations_mutex);
            return read_size;
        }

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

            if (fat_clus_entry_val == CLUSTER_END) {
                log_printf("[END] Read succeeded for path %s, %lu bytes read.\n",
                           path, read_size);
                up(&file_operations_mutex);
                return read_size;
            }

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

    log_printf("[END] Read succeeded for path %s, %lu bytes read.\n", path,
               read_size);
    up(&file_operations_mutex);
    return read_size;
    /*** END ***/
}

/**
 * @brief 在 path 对应的路径创建新文件
 *
 * @param path    要创建的文件路径
 * @param mode    要创建文件的类型, 本次实验可忽略, 默认所有创建的文件都为普通文件
 * @param devNum  忽略, 要创建文件的设备的设备号
 * @return int    成功返回 0, 失败返回 POSIX 错误代码的负值
 */
int fat16_mknod(const char *path, mode_t mode, dev_t devNum)
{
    down(&file_operations_mutex);
    log_printf("[BEGIN] Mknod called for path %s.\n", path);

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

        if (find_root(fat16_ins, &Dir, prtPath, &offset_dir)) {
            log_printf("[ERROR] Parent directory %s not found.\n", prtPath);
            log_printf("[END] Mknod failed for path %s.\n", path);
            up(&file_operations_mutex);
            return -ENOENT;
        }
        if (Dir.DIR_Attr == ATTR_ARCHIVE) {
            log_printf("[ERROR] Parent directory %s is a file.\n", prtPath);
            log_printf("[END] Mknod failed for path %s.\n", path);
            up(&file_operations_mutex);
            return -ENOTDIR;
        }

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
        // 正确调用 dir_entry_create 创建目录项
        /*** BEGIN ***/
        dir_entry_create(fat16_ins, sectorNum, offset, paths[pathDepth - 1],
                         ATTR_ARCHIVE, CLUSTER_END, 0);
        /*** END ***/
    }

    log_printf("[END] Mknod succeeded for path %s.\n", path);
    up(&file_operations_mutex);
    return 0;
}

/**
 * @brief 删除 path 对应的文件
 *
 * @param path  要删除的文件路径
 * @return int  成功返回 0, 失败返回 POSIX 错误代码的负值
 */
int fat16_unlink(const char *path)
{
    down(&file_operations_mutex);
    log_printf("[BEGIN] Unlink called for path %s.\n", path);

    FAT16 *fat16_ins = get_fat16_ins();

    DIR_ENTRY Dir;
    off_t     offset_dir;
    // 释放使用过的簇
    if (find_root(fat16_ins, &Dir, path, &offset_dir) == 1) {
        log_printf("[ERROR] File %s not found.\n", path);
        log_printf("[END] Unlink failed for path %s.\n", path);
        up(&file_operations_mutex);
        return -ENOENT;
    }
    if (Dir.DIR_Attr == ATTR_DIRECTORY) {
        log_printf("[ERROR] File %s is a directory.\n", path);
        log_printf("[END] Unlink failed for path %s.\n", path);
        up(&file_operations_mutex);
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
        if (find_root(fat16_ins, &Dir, prtPath, &offset_dir)) {
            log_printf("[ERROR] Parent directory %s not found.\n", prtPath);
            log_printf("[END] Unlink failed for path %s.\n", path);
            up(&file_operations_mutex);
            return -ENOENT;
        }
        if (Dir.DIR_Attr == ATTR_ARCHIVE) {
            log_printf("[ERROR] Parent directory %s is a file.\n", prtPath);
            log_printf("[END] Unlink failed for path %s.\n", path);
            up(&file_operations_mutex);
            return -ENOTDIR;
        }

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

    log_printf("[END] Unlink succeeded for path %s.\n", path);
    up(&file_operations_mutex);
    return 0;
}

/**
 * @brief 创建 path 对应的文件夹
 *
 * @param path 创建的文件夹路径
 * @param mode 文件模式, 本次实验可忽略, 默认都为普通文件夹
 * @return int 成功:0, 失败: POSIX 错误代码的负值
 */
int fat16_mkdir(const char *path, mode_t mode)
{
    down(&file_operations_mutex);
    log_printf("[BEGIN] mkdir called for path %s.\n", path);

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
        if (find_root(fat16_ins, &dir, prtPath, &dirOffset)) {
            log_printf("[ERROR] Parent directory %s not found.\n");
            log_printf("[END] Mkdir failed for path %s.\n", path);
            up(&file_operations_mutex);
            return -ENOENT;
        } else if (dir.DIR_Attr == ATTR_ARCHIVE) {
            log_printf("[ERROR] Parent directory %s is a file.\n");
            log_printf("[END] Mkdir failed for path %s.\n", path);
            up(&file_operations_mutex);
            return -ENOTDIR;
        }

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
    // 在父目录的目录项中添加新建的目录. 同时, 为新目录分配一个簇,
    // 并在这个簇中创建两个 目录项, 分别指向. 和 .., 目录的文件大小设置为 0 即可.
    if (findFlag == 1) {
        /*** BEGIN ***/
        WORD allocatedClus = alloc_clusters(fat16_ins, 1);
        if (allocatedClus == CLUSTER_END) {
            log_printf("[ERROR] No enough space.\n");
            log_printf("[END] Mkdir failed for path %s.\n", path);
            up(&file_operations_mutex);
            return -ENOSPC;
        }
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

    log_printf("[END] Mkdir succeeded for path %s.\n", path);
    up(&file_operations_mutex);
    return 0;
}

/**
 * @brief 写入 offset 位置的目录项
 *
 * @param fat16_ins 文件系统指针
 * @param offset    find_root 传回的 offset_dir 值
 * @param Dir       要写入的目录项
 */
int fat16_rmdir(const char *path)
{
    down(&file_operations_mutex);
    log_printf("[BEGIN] Rmdir called for path %s.\n", path);

    FAT16 *fat16_ins = get_fat16_ins_fix();

    if (strcmp(path, "/") == 0) {
        log_printf("[ERROR] Cannot remove root directory.\n");
        log_printf("[END] Rmdir failed for path %s.\n", path);
        up(&file_operations_mutex);
        return -EBUSY;  // 无法删除根目录, 根目录是挂载点 (可参考 `man 2 rmdir`)
    }

    DIR_ENTRY dir;
    DIR_ENTRY curDir;
    off_t     offset;

    if (find_root(fat16_ins, &dir, path, &offset)) {
        log_printf("[ERROR] Path %s not found.\n", path);
        log_printf("[END] Rmdir failed for path %s.\n", path);
        up(&file_operations_mutex);
        return -ENOENT;  // 路径不存在
    }
    if (dir.DIR_Attr != ATTR_DIRECTORY) {
        log_printf("[ERROR] Path %s is a file.\n", path);
        log_printf("[END] Rmdir failed for path %s.\n", path);
        up(&file_operations_mutex);
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
                log_printf("[ERROR] Directory %s is not empty.\n", path);
                log_printf("[END] Rmdir failed for path %s.\n", path);
                up(&file_operations_mutex);
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

    log_printf("[END] Rmdir succeeded for path %s.\n", path);
    up(&file_operations_mutex);
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
    down(&file_operations_mutex);
    log_printf(
        "[BEGIN] Write called for path %s, size %lu, offset %ld and data %.30s.\n",
        path, size, offset, data);

    FAT16    *fat16_ins = get_fat16_ins_fix();
    DIR_ENTRY dir;
    off_t     offsetDir;
    if (find_root(fat16_ins, &dir, path, &offsetDir)) {
        log_printf("[ERROR] File %s not found.\n", path);
        log_printf("[END] Write failed for path %s.\n", path);
        up(&file_operations_mutex);
        return -ENOENT;
    }

    int result = write_file(fat16_ins, &dir, offsetDir, data, offset, size);
    if (result < 0) {
        switch (result) {
            {
            case -EINVAL:
                log_printf("[ERROR] Invalid argument.\n");
                break;
            case -ENOSPC:
                log_printf("[ERROR] No enough space.\n");
                break;

            default:
                log_printf("[ERROR] Unknown error, POSIX error code %d.\n", -result);
                break;
            }
            log_printf("[END] Write failed for path %s.\n", path);
            up(&file_operations_mutex);
            return result;
        }
    }

    log_printf("[END] Write succeeded for path %s.\n", path);
    up(&file_operations_mutex);
    return result;
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
    down(&file_operations_mutex);
    log_printf("[BEGIN] Truncate called for path %s, size %ld.\n", path, size);
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
        log_printf("[END] Truncate succeeded for path %s.\n", path);
        up(&file_operations_mutex);
        return 0;
    } else if (oldSize < newSize) {
        /** TODO: 增大文件大小, 注意是否需要分配新簇, 以及往新分配的空间填充 0 等 **/
        /*** BEGIN ***/
        if (newClusCnt > curClusCnt) {
            int ret =
                file_new_cluster(fat16_ins, &dir, lastClus, newClusCnt - curClusCnt);
            if (ret < 0) {
                log_printf("[ERROR] No enough space.\n");
                log_printf("[END] Truncate failed for path %s.\n", path);
                up(&file_operations_mutex);
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
            while (is_cluster_inuse(firstFreeClus)) {
                firstFreeClus = free_cluster(fat16_ins, firstFreeClus);
            }
        }
        /*** END ***/
    }
    // 写回文件大小
    dir.DIR_FileSize = newSize;
    dir_entry_write(fat16_ins, offsetDir, &dir);

    log_printf("[END] Truncate succeeded for path %s.\n", path);
    up(&file_operations_mutex);
    return 0;
}