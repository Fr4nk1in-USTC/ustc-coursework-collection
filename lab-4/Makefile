CFLAGS=$(shell pkg-config fuse --cflags) -g -Wall -std=gnu99 -Wno-unused-variable
LDFLAGS=$(shell pkg-config fuse --libs)
LDLIBS=

CC=gcc

all: simple_fat16

default: simple_fat16_part1.o simple_fat16_part2.o
	$(CC) $(CFLAGS) -o simple_fat16 $^ $(LDFLAGS) $(LDLIBS)

backup: simple_fat16_part1.o simple_fat16_part2.o simple_fat16_backup.o
	$(CC) $(CFLAGS) -o simple_fat16 $^ $(LDFLAGS) $(LDLIBS)

log: simple_fat16_part1.o simple_fat16_part2.o simple_fat16_log.o
	$(CC) $(CFLAGS) -o simple_fat16 $^ $(LDFLAGS) $(LDLIBS)

simple_fat16_part1.o: simple_fat16_part1.c fat16.h
	$(CC) $(CFLAGS) -c -o $@ $<

simple_fat16_part2.o: simple_fat16_part2.c fat16.h
	$(CC) $(CFLAGS) -c -o $@ $<

simple_fat16_backup.o: simple_fat16_backup.c fat16.h backup.h
	$(CC) $(CFLAGS) -c -o $@ $<

simple_fat16_log.o: simple_fat16_log.c fat16.h log.h
	$(CC) $(CFLAGS) -c -o $@ $<

simple_fat16_test.o: simple_fat16_test.c fat16.h
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f simple_fat16 *.o *.log
