#include <algorithm>
#include <fstream>
#include <random>
#include <vector>

using std::endl;
using std::mt19937;
using std::ofstream;
using std::random_device;
using std::shuffle;
using std::uniform_int_distribution;
using std::vector;

#define INPUT_FILE   "../input/input.txt"
#define INTERVAL_NUM 30
#define LOW_1        0
#define HIGH_1       25
#define LOW_2        30
#define HIGH_2       50

int main()
{
    vector<int>   low, high;
    random_device rd;
    mt19937       gen(rd());
    // Generate lower and higher bounds of intervals
    int length = HIGH_1 - LOW_1 + HIGH_2 - LOW_2;
    for (int i = 0; i < length; ++i)
        low.push_back(i);
    shuffle(low.begin(), low.end(), gen);
    low.resize(INTERVAL_NUM);
    for (auto &i : low) {
        int max;
        if (i >= HIGH_1) {
            i   += LOW_2 - HIGH_1;
            max = HIGH_2;
        } else {
            max = HIGH_1;
        }
        high.push_back(uniform_int_distribution<int>(i, max)(gen));
    }
    // Output to file
    ofstream fout(INPUT_FILE);
    for (int i = 0; i < INTERVAL_NUM; ++i)
        fout << low[i] << ' ' << high[i] << endl;
    fout.close();
}
