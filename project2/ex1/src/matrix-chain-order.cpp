#include <bits/types/FILE.h>
#include <chrono>
#include <cstdio>
#include <cstring>

using std::chrono::duration_cast;
using ns = std::chrono::nanoseconds;
auto now = std::chrono::high_resolution_clock::now;

#define INPUT_FILE  "../input/2_1_input.txt"
#define RESULT_FILE "../output/result.txt"
#define TIME_FILE   "../output/time.txt"

#define MAX_N 25
#define INFTY 9223372036854775807  // 1 << 63 - 1

long long    m[MAX_N + 1][MAX_N + 1];
unsigned int s[MAX_N][MAX_N + 1];

/**
 * @brief Calculate the minimum cost and optimal order of matrix chain
 *        multiplication using dynamic programming.
 *        The minimum cost and optimal order is stored in global table
 *        `m` and `s`.
 *        The minimum cost of the whole chain is stored in `m[1][n]`.
 * @param p The scale of each matrix. The scale of matrix i is p[i-1] * p[i]
 * @param n The number of matrices
 */
void matrix_chain_order(long long p[], unsigned int n)
{
    for (unsigned int i = 1; i <= n; ++i)
        m[i][i] = 0;
    for (unsigned int l = 2; l <= n; ++l) {
        for (unsigned int i = 1; i <= n - l + 1; ++i) {
            unsigned int j = i + l - 1;
            m[i][j]        = INFTY;
            for (unsigned int k = i; k < j; ++k) {
                long long q = m[i][k] + m[k + 1][j] + p[i - 1] * p[k] * p[j];
                if (q < m[i][j]) {
                    m[i][j] = q;
                    s[i][j] = k;
                }
            }
        }
    }
}

/**
 * @brief Print the optimal parenthesis string to file for sub-problem A`i`
 *        to A`j` using the global table `s`.
 *        Should be called after `matrix_chain_order()`.
 *        Initial call should be `print_optimal_parens(1, n, out)`.
 * @param i   Starting index of sub-problem.
 * @param j   Ending index of sub-problem.
 * @param out The file to print the optimal parenthesis string.
 */
void print_optimal_parens(unsigned int i, unsigned int j, FILE *out)
{
    if (i == j) {
        fprintf(out, "A%u", i);
    } else {
        fprintf(out, "(");
        print_optimal_parens(i, s[i][j], out);
        print_optimal_parens(s[i][j] + 1, j, out);
        fprintf(out, ")");
    }
}

int main()
{
    FILE *input_fp, *result_fp, *time_fp;
    input_fp  = fopen(INPUT_FILE, "r");
    result_fp = fopen(RESULT_FILE, "w");
    time_fp   = fopen(TIME_FILE, "w");

    unsigned int n;
    long long    p[MAX_N + 1];

    // Flush cache
    memset(m, 0, sizeof(m));
    memset(s, 0, sizeof(s));

    while (fscanf(input_fp, "%u", &n) != EOF) {
        // Scan input
        for (unsigned int i = 0; i <= n; ++i)
            fscanf(input_fp, "%lld", &p[i]);

        // Set timer and evaluate duration
        auto begin_time = now();
        matrix_chain_order(p, n);
        auto end_time = now();
        long duration = duration_cast<ns>(end_time - begin_time).count();

        // Print output to file
        fprintf(result_fp, "%lld\n", m[1][n]);
        print_optimal_parens(1, n, result_fp);
        fprintf(result_fp, "\n");
        fprintf(time_fp, "%2u: %ld ns\n", n, duration);

        // Print m, s, optimal parenthesis and minimal cost to console if n == 5
        if (n == 5) {
            printf("For n == 5\n");

            printf("Matrix m:\n");
            printf(" m |");
            for (unsigned int i = 1; i <= n; ++i)
                printf("%16u", i);
            printf("\n---+-----------------------------------------------------"
                   "----------------------------\n");
            for (unsigned int j = n; j > 0; --j) {
                printf(" %u |", j);
                for (unsigned int i = 1; i <= j; ++i)
                    printf("%16lld", m[i][j]);
                putchar('\n');
            }

            printf("Matrix s:\n");
            printf(" s | 1 2 3 4\n");
            printf("---+---------\n");
            for (unsigned int j = n; j > 1; --j) {
                printf(" %u |", j);
                for (unsigned int i = 1; i < j; ++i)
                    printf("%2u", s[i][j]);
                putchar('\n');
            }

            printf("Optimal parenthesis: \n");
            print_optimal_parens(1, n, stdout);
            putchar('\n');

            printf("Cost: %lld\n", m[1][n]);
        }
    }
    fclose(input_fp);
    fclose(result_fp);
    fclose(time_fp);
}
