#include "config.h"

#include <fstream>
#include <random>

using std::endl;
using std::mt19937;
using std::ofstream;
using std::random_device;
using std::uniform_int_distribution;

int main()
{
    ofstream      fout(INPUT_FILE);
    random_device rd;
    mt19937       gen(rd());

    uniform_int_distribution<int> dis(min_val, max_val);
    for (int i = 0; i < size; ++i) {
        fout << dis(gen) << endl;
    }
    fout.close();
    return 0;
}
