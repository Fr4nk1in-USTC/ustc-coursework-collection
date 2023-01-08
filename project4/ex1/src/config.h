#ifndef CONFIG_H
#define CONFIG_H

const char input_files[4][2][21] = {
    {"../input/input11.txt", "../input/input12.txt"},
    {"../input/input21.txt", "../input/input22.txt"},
    {"../input/input31.txt", "../input/input32.txt"},
    {"../input/input41.txt", "../input/input42.txt"}
};

const char result_files[4][2][23] = {
    {"../output/result11.txt", "../output/result12.txt"},
    {"../output/result21.txt", "../output/result22.txt"},
    {"../output/result31.txt", "../output/result32.txt"},
    {"../output/result41.txt", "../output/result42.txt"}
};

const char time_file[] = "../output/time.txt";

const int inf  = 50 * 729 + 1;
const int ninf = -2147483648;

const int num_vertices[] = {27, 81, 243, 729};

const int num_edges[][2] = {
    {2, 1},
    {2, 2},
    {3, 2},
    {4, 3}
};
#endif
