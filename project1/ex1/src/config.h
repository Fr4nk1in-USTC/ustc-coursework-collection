#ifndef __CONFIG_H__
#define __CONFIG_H__

const int max_val = (1 << 15) - 1;
const int min_val = 0;
const int inf     = 0x7FFFFFFF;

const int size = 1 << 18;

const int exps[] = {3, 6, 9, 12, 15, 18};
const int n_exps = sizeof(exps) / sizeof(int);

#define INPUT_FILE "../input/input.txt"
#define OUTPUT_DIR "../output/"

#endif
