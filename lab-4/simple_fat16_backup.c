#include "fat16.h"
#include <string.h>

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
                        "Find uncompatible data at sector %u, byte %d, "
                        "bit %d\n",
                        secnum, i, j);
                correct_bit = (origin_bit + backup_bit[0] + backup_bit[1]) >> 1;
                fprintf(stderr,
                        "\tCorrect bit %1d, origin bit %1d, "
                        "backup bit %1d & %1d\n",
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