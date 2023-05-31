#include <bitset>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using std::cout;
using std::endl;
using std::greater;
using std::ifstream;
using std::invalid_argument;
using std::istream;
using std::make_shared;
using std::ofstream;
using std::ostream;
using std::priority_queue;
using std::shared_ptr;
using std::unordered_map;
using std::unordered_set;
using std::vector;

const size_t MAX_SIZE = 12;
const size_t CASE_NUM = 10;

auto now     = std::chrono::high_resolution_clock::now;
using ns     = std::chrono::nanoseconds;
using grid_t = std::bitset<MAX_SIZE * MAX_SIZE>;

const char *INPUTS[] = {"../input/input0.txt", "../input/input1.txt",
                        "../input/input2.txt", "../input/input3.txt",
                        "../input/input4.txt", "../input/input5.txt",
                        "../input/input6.txt", "../input/input7.txt",
                        "../input/input8.txt", "../input/input9.txt"};

const char *OUTPUTS[] = {"../output/output0.txt", "../output/output1.txt",
                         "../output/output2.txt", "../output/output3.txt",
                         "../output/output4.txt", "../output/output5.txt",
                         "../output/output6.txt", "../output/output7.txt",
                         "../output/output8.txt", "../output/output9.txt"};

/** Class representing an L-flip action.
 * The shape of the L-flip action is a value between 0 and 3, each value
 * corresponds to a different L-shape:
 * - 1: (x, y), (x - 1, y), (x, y + 1) | └-shape
 * - 2: (x, y), (x - 1, y), (x, y - 1) | ┘-shape
 * - 3: (x, y), (x + 1, y), (x, y - 1) | ┐-shape
 * - 4: (x, y), (x + 1, y), (x, y + 1) | ┌-shape
 */
class action
{
  public:
    unsigned x;
    unsigned y;
    unsigned shape;

    action(): x(0), y(0), shape(0) {}

    action(unsigned x_, ushort y_, ushort s): x(x_), y(y_), shape(s) {}

    friend ostream &operator<<(ostream &os, const action &a)
    {
        os << a.x << ',' << a.y << ',' << a.shape;
        return os;
    }
};

unsigned heuristic(const grid_t &g)
{
    int count = g.count();
    return count / 3 + count % 3;
}

/* The heuristic below should be consistent, but I have no time to prove it */
unsigned better_heuristic(const grid_t &g, ushort size)
{
    vector<vector<unsigned>> dist(size, vector<unsigned>(size, 0));
    // Iterate over the grid to find all 1s with other 1s surrounded and mark it
    for (unsigned i = 0; i < size - 1; ++i) {
        for (unsigned j = 0; j < size - 1; ++j) {
            unsigned count = g[i * size + j] + g[i * size + j + 1]
                           + g[(i + 1) * size + j] + g[(i + 1) * size + j + 1];
            if (count >= 3) {
                dist[i][j]         |= 3;
                dist[i + 1][j]     |= 3;
                dist[i][j + 1]     |= 3;
                dist[i + 1][j + 1] |= 3;
            } else if (count == 2) {
                dist[i][j]         |= 2;
                dist[i + 1][j]     |= 2;
                dist[i][j + 1]     |= 2;
                dist[i + 1][j + 1] |= 2;
            }
        }
    }
    // Count the number of 1s, 2s and 3s
    float counts[3] = {0, 0, 0};
    for (unsigned i = 0; i < size; ++i) {
        for (unsigned j = 0; j < size; ++j) {
            if (g[i * size + j]) {
                if (dist[i][j]) {
                    ++counts[dist[i][j] - 1];
                } else {
                    ++counts[0];
                }
            }
        }
    }

    unsigned value = ceil(counts[0] + 7 * counts[1] / 15 + counts[2] / 3);
    unsigned sum   = g.count();

    if ((value & 1) == (sum & 1)) {
        return value;
    } else {
        return value + 1;
    }
}

class node
{
  public:
    shared_ptr<node> parent;       // parent node
    action           from_parent;  // action from parent
    unsigned         g;            // steps from the root node
    unsigned         h;            // heuristic value
    unsigned         f;            // h + g
    grid_t           grid;         // grid of the current node

    // constructor for root node
    node(const grid_t &g_, const unsigned h_):
        parent(nullptr),
        grid(g_),
        g(0),
        h(h_),
        f(h_)
    {}

    // constructor for non-root node
    node(const shared_ptr<node> &p, const action &a, const grid_t &g_,
         const unsigned h_):
        parent(p),
        from_parent(a),
        grid(g_),
        g(p->g + 1),
        f(p->g + 1 + h_),
        h(h_)
    {}
};

bool operator<(const shared_ptr<node> &lhs, const shared_ptr<node> &rhs)
{
    return lhs->f < rhs->f or (lhs->f == rhs->f and lhs->g > rhs->g);
}

class solver
{
  public:
    solver() {}

    solver from_file(istream &file)
    {
        file >> size;

        if (size < 2) {
            throw invalid_argument("The input grid is too small");
        }

        for (unsigned i = 0; i < size * size; ++i) {
            unsigned temp;
            file >> temp;
            grid[i] = temp;
        }

        return *this;
    }

    void solve()
    {
        auto root = make_shared<node>(grid, heuristic(grid));

        using node_p = shared_ptr<node>;

        priority_queue<node_p, vector<node_p>, greater<node_p>> frontier;
        frontier.push(root);

        unordered_set<grid_t> explored;

        while (not frontier.empty()) {
            auto current = frontier.top();
            frontier.pop();

            if (current->h == 0) {
                set_solution(current);
                return;
            }

            explored.insert(current->grid);

            for (const auto &[g, a] : get_childrens(current->grid)) {
                auto child = make_shared<node>(current, a, g, heuristic(g));

                if (explored.find(g) == explored.end()) {  // not explored
                    frontier.push(child);
                }
            }
        }

        // This should never be reached
        return;
    }

    bool verify()
    {
        grid_t g           = grid;
        auto   take_action = [&g](const action &a, unsigned size) {
            unsigned x   = a.x * size;
            unsigned y   = a.y;
            unsigned x_1 = x + ((a.shape < 3) ? -size : size);
            unsigned y_1 = y + ((a.shape == 2 or a.shape == 3) ? -1 : 1);

            g[x + y]   = !g[x + y];
            g[x_1 + y] = !g[x_1 + y];
            g[x + y_1] = !g[x + y_1];
        };

        for (auto &a : path) {
            take_action(a, size);
        }

        return g.count() == 0;
    }

    void path_to(ostream &file)
    {
        file << path.size() << endl;
        for (auto &a : path) {
            file << a << endl;
        }
    }

    vector<action> get_path() { return path; }

  private:
    grid_t         grid;
    unsigned       size;
    vector<action> path;  // optimal path

    unordered_map<grid_t, action> get_childrens(grid_t &g)
    {
        unsigned first_one_index;
        for (unsigned i = 0; i < size * size; ++i) {
            if (g[i]) {
                first_one_index = i;
                break;
            }
        }

        unsigned x = first_one_index / size;
        unsigned y = first_one_index % size;

        // 12 possible actions on the 1
        // +-------+-------+-------+-------+-------+-------+
        // | # # 0 | # 0 0 | 0 # 0 | 0 # 0 | 0 # # | 0 0 # |
        // | 0 # 0 | # # 0 | # # 0 | 0 # # | 0 # 0 | 0 # # |
        // | 0 0 0 | 0 0 0 | 0 0 0 | 0 0 0 | 0 0 0 | 0 0 0 |
        // +-------+-------+-------+-------+-------+-------+
        // | 0 0 0 | 0 0 0 | 0 0 0 | 0 0 0 | 0 0 0 | 0 0 0 |
        // | 0 # 0 | 0 # # | 0 # # | # # 0 | # # 0 | 0 # 0 |
        // | 0 # # | 0 0 # | 0 # 0 | 0 # 0 | # 0 0 | # # 0 |
        // +-------+-------+-------+-------+-------+-------+
        // If the cell is not on the last row of grid, only the last 6 actions
        // are effective. If the cell is on the last row, then only the first
        // 6 actions are valid. Thus, only up to 6 actions will be generated.
        unordered_map<grid_t, action> actions_to_grids;

        static int8_t   pos_x_offset[] = {-1, 0, 0, 0, -1, 0, 1, 0, 0, 0, 0, 1};
        static int8_t   pos_y_offset[] = {0, -1, 0, 0, 0, 1, 0, 1, 0, 0, -1, 0};
        static int8_t   arm_x_offset[] = {1,  -1, -1, -1, 1, -1,
                                          -1, 1,  1,  1,  1, -1};
        static int8_t   arm_y_offset[] = {-1, 1,  -1, 1,  1, -1,
                                          1,  -1, 1,  -1, 1, -1};
        static unsigned shapes[]       = {3, 1, 2, 1, 4, 2, 1, 3, 4, 3, 4, 2};

        auto gen = [&actions_to_grids, &g, x, y, this](unsigned i) {
            unsigned pos_x = x + pos_x_offset[i];
            unsigned pos_y = y + pos_y_offset[i];
            unsigned arm_x = pos_x + arm_x_offset[i];
            unsigned arm_y = pos_y + arm_y_offset[i];

            grid_t new_g = g;
            action a(pos_x, pos_y, shapes[i]);

            // Take action
            pos_x *= size;
            arm_x *= size;

            new_g.flip(pos_x + pos_y);
            new_g.flip(arm_x + pos_y);
            new_g.flip(pos_x + arm_y);

            actions_to_grids[new_g] = a;
        };

        if (x != size - 1) {
            if (y != 0) {
                gen(9);
                gen(10);
                gen(11);
            }
            if (y != size - 1) {
                gen(6);
                gen(7);
                gen(8);
            }
        } else {
            if (y != 0) {
                gen(0);
                gen(1);
                gen(2);
            }
            if (y != size - 1) {
                gen(3);
                gen(4);
                gen(5);
            }
        }

        return actions_to_grids;
    }

    void set_solution(const shared_ptr<node> &goal)
    {
        path.resize(goal->g);
        unsigned         index   = 0;
        shared_ptr<node> current = goal;
        // reconstruct the path, order does not matter
        while (current->parent != nullptr) {
            path[index++] = current->from_parent;
            current       = current->parent;
        }
    }
};

int main()
{
    for (int i = 0; i < CASE_NUM; i++) {
        ifstream input(INPUTS[i]);

        solver s = solver().from_file(input);

        auto start = now();
        s.solve();
        auto end = now();

        ns duration = end - start;
        cout << "Time taken for " << INPUTS[i] << ": " << duration.count() / 1e6
             << " ms, "
             << "solution verified: " << s.verify() << endl;

        ofstream output(OUTPUTS[i]);
        s.path_to(output);
    }

    return 0;
}
