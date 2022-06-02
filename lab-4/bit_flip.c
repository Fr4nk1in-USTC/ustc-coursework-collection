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