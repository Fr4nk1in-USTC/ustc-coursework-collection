#include <bits/types/FILE.h>
#include <chrono>
#include <cstdio>
#include <cstring>

using std::chrono::duration_cast;
using ns = std::chrono::nanoseconds;
auto now = std::chrono::high_resolution_clock::now;

#define INPUT_FILE    "../input/2_2_input.txt"
#define RESULT_PREFIX "../output/result_"
#define RESULT_SUFFIX ".txt"
#define TIME_FILE     "../output/time.txt"
#define RESULT_FN_LEN 23

#define MAX_N 30

enum arrow {
    UP,
    LEFT,
    UP_LEFT
};

unsigned int c[MAX_N + 1][MAX_N + 1];
arrow        b[MAX_N + 1][MAX_N + 1];

/**
 * @brief Calculate the length of longest common subsequence using dynamic
 *        programming.
 *        The length and arrow direction of sub-problems is stored in global
 *        table `c` and `b`.
 *        The length of LCS of x and y is stored in c[m][n].
 * @param x The first string
 * @param y The second string
 * @param m The length of x
 * @param n The length of y
 */
void lcs_length(const char x[], const char y[], unsigned int m, unsigned int n)
{
    for (unsigned int i = 1; i <= m; ++i)
        c[i][0] = 0;
    for (unsigned int j = 0; j <= n; ++j)
        c[0][j] = 0;

    for (unsigned int i = 1; i <= m; ++i) {
        for (unsigned int j = 1; j <= n; ++j) {
            if (x[i - 1] == y[j - 1]) {
                c[i][j] = c[i - 1][j - 1] + 1;
                b[i][j] = UP_LEFT;
            } else if (c[i - 1][j] >= c[i][j - 1]) {
                c[i][j] = c[i - 1][j];
                b[i][j] = UP;
            } else {
                c[i][j] = c[i][j - 1];
                b[i][j] = LEFT;
            }
        }
    }
}

/**
 * @brief Print the longest common subsequence to file using the global
 *        table `b`.
 *        Should be called after `lcs_length()`.
 *        Initial call should be `print_lcs(x, m, n, out)`.
 * @param x The first string
 * @param i First index
 * @param j Second index
 * @param out The file to print the longest common subsequence
 */
void print_lcs(const char *x, unsigned int i, unsigned int j, FILE *out)
{
    if (i == 0 || j == 0)
        return;

    if (b[i][j] == UP_LEFT) {
        print_lcs(x, i - 1, j - 1, out);
        fputc(x[i - 1], out);
    } else if (b[i][j] == UP) {
        print_lcs(x, i - 1, j, out);
    } else {
        print_lcs(x, i, j - 1, out);
    }
}

int main()
{
    FILE *input_fp, *result_fp, *time_fp;
    input_fp = fopen(INPUT_FILE, "r");
    time_fp  = fopen(TIME_FILE, "w");

    unsigned int n;
    char         x[MAX_N], y[MAX_N];
    char         result_filename[RESULT_FN_LEN];

    // Flush cache
    memset(b, UP, sizeof(b));
    memset(c, 0, sizeof(c));

    while (fscanf(input_fp, "%u", &n) != EOF) {
        // Scan input
        fscanf(input_fp, "%s", x);
        fscanf(input_fp, "%s", y);

        // Set timer and evaluate duration
        auto begin_time = now();
        lcs_length(x, y, n, n);
        auto end_time = now();
        long duration = duration_cast<ns>(end_time - begin_time).count();

        // Print result to file and console
        // Open result file and print lcs length and lcs
        sprintf(result_filename, RESULT_PREFIX "%u" RESULT_SUFFIX, n);
        result_fp = fopen(result_filename, "w");
        fprintf(result_fp, "%u\n", c[n][n]);
        print_lcs(x, n, n, result_fp);
        fprintf(result_fp, "\n");
        fclose(result_fp);
        // Print duration to time file
        fprintf(time_fp, "%2u: %ld ns\n", n, duration);
        // Print lcs length and lcs to console
        printf("n = %2u: length = %2u, lcs = ", n, c[n][n]);
        print_lcs(x, n, n, stdout);
        putchar('\n');
    }
    fclose(input_fp);
    fclose(time_fp);
}
