#include "interval-tree.h"

#include <algorithm>
#include <fstream>
#include <random>
#include <string>
#include <vector>

using std::endl;
using std::ifstream;
using std::max;
using std::min;
using std::mt19937;
using std::ofstream;
using std::random_device;
using std::shuffle;
using std::string;
using std::to_string;
using std::uniform_int_distribution;
using std::vector;

#define INTERVAL_NUM 30
#define REMOVE_NUM   3
#define QUERY_NUM    3

#define INTERVAL_MIN 0
#define INTERVAL_MAX 50
#define GAP_MIN      26
#define GAP_MAX      29

#define INPUT_FILE   "../input/input.txt"
#define INORDER_FILE "../output/inorder.txt"
#define DELETE_FILE  "../output/delete_data.txt"
#define SEARCH_FILE  "../output/search.txt"

// Output interval tree into a LaTeX file if in debug mode
#ifdef DEBUG
    #define TIKZ_FILE_PREFIX "../latex/tree-"

void output_tikz(const IntervalTree &tree, const string &suffix)
{
    ofstream tikz_file(TIKZ_FILE_PREFIX + suffix + ".tex");
    tree.to_tikz(tikz_file);
    tikz_file.close();
}
#else
    #define output_tikz(...) ;
#endif

int main()
{
    IntervalTree tree;
    // Construct interval tree from input
    ifstream    fin(INPUT_FILE);
    vector<int> low(INTERVAL_NUM);
    vector<int> high(INTERVAL_NUM);
    for (int i = 0; i < INTERVAL_NUM; ++i) {
        fin >> low[i] >> high[i];
        tree.insert(low[i], high[i]);
        // output_tikz(tree, "insert-" + to_string(i));
    }
    output_tikz(tree, "original");
    fin.close();
    ofstream inorder_file(INORDER_FILE);
    tree.inorder_output(inorder_file);
    inorder_file.close();

    // Remove 3 nodes randomly
    vector<int> delete_index(INTERVAL_NUM);
    for (int i = 0; i < INTERVAL_NUM; ++i)
        delete_index[i] = i;

    random_device rd;
    mt19937       gen(rd());
    shuffle(delete_index.begin(), delete_index.end(), gen);

    ofstream delete_file(DELETE_FILE);
    for (int i = 0; i < REMOVE_NUM; ++i) {
        int index = delete_index[i];

        IntervalNode *removed = tree.remove(low[index], high[index]);
        delete_file << removed->low << ' ' << removed->high << ' '
                    << removed->max << endl;
        // output_tikz(tree, "remove-" + to_string(i) + "-" +
        // to_string(removed->low) + "-" + to_string(removed->high));
        delete removed;
    }
    output_tikz(tree, "removed");
    delete_file << endl;
    tree.inorder_output(delete_file);
    delete_file.close();

    // Search 3 intervals randomly, one of which is in the gap
    vector<int> search_low(QUERY_NUM);
    vector<int> search_high(QUERY_NUM);

    uniform_int_distribution<int> interval_dist(INTERVAL_MIN, INTERVAL_MAX);
    uniform_int_distribution<int> gap_dist(GAP_MIN, GAP_MAX);

    for (int i = 0; i < QUERY_NUM - 1; ++i) {
        int bound_1    = interval_dist(gen);
        int bound_2    = interval_dist(gen);
        search_low[i]  = min(bound_1, bound_2);
        search_high[i] = max(bound_1, bound_2);
    }
    int gap_1 = gap_dist(gen);
    int gap_2 = gap_dist(gen);

    search_low[QUERY_NUM - 1]  = min(gap_1, gap_2);
    search_high[QUERY_NUM - 1] = max(gap_1, gap_2);

    ofstream search_file(SEARCH_FILE);
    for (int i = 0; i < QUERY_NUM; ++i) {
        IntervalNode *result = tree.search(search_low[i], search_high[i]);
        search_file << search_low[i] << ' ' << search_high[i] << " -> ";
        if (result == nullptr)
            search_file << "null" << endl;
        else
            search_file << result->low << ' ' << result->high << ' '
                        << result->max << endl;
    }
    search_file.close();
}
