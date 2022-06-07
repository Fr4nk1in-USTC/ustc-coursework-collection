#include <stdio.h>

extern const char *log_filename;
extern FILE       *log_fd;

typedef int mutex;

extern mutex file_operations_mutex;
extern mutex file_read_write_mutex;
extern mutex reader_count_mutex;
extern mutex log_mutex;

extern int reader_count;

void log_open();
void log_printf(const char *format, ...);

void down(mutex *m);
void up(mutex *m);