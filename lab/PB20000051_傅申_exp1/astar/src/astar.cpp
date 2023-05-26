#include <bitset>
#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <vector>

auto now = std::chrono::high_resolution_clock::now;
using ns = std::chrono::nanoseconds;

using std::bitset;
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

using grid_t = bitset<MAX_SIZE * MAX_SIZE>;
using ushort = unsigned short;

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
 * - 0: (x, y), (x - 1, y), (x, y + 1) | └-shape
 * - 1: (x, y), (x - 1, y), (x, y - 1) | ┘-shape
 * - 2: (x, y), (x + 1, y), (x, y - 1) | ┐-shape
 * - 3: (x, y), (x + 1, y), (x, y + 1) | ┌-shape
 */
class action
{
  public:
    ushort x;
    ushort y;
    ushort shape;

    action(): x(0), y(0), shape(0) {}

    action(ushort x_, ushort y_, ushort s): x(x_), y(y_), shape(s) {}

    friend ostream &operator<<(ostream &os, const action &a)
    {
        os << a.x << ',' << a.y << ',' << a.shape;
        return os;
    }
};

ushort heuristic(const grid_t &g, ushort size)
{
    vector<vector<unsigned>> dist(size, vector<unsigned>(size, 0));
    // Iterate over the grid to find all 1s with other 1s surrounded and mark it
    for (ushort i = 0; i < size - 1; ++i) {
        for (ushort j = 0; j < size - 1; ++j) {
            ushort count = g[i * size + j] + g[i * size + j + 1]
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
    for (ushort i = 0; i < size; ++i) {
        for (ushort j = 0; j < size; ++j) {
            if (g[i * size + j]) {
                if (dist[i][j]) {
                    ++counts[dist[i][j] - 1];
                } else {
                    ++counts[0];
                }
            }
        }
    }

    ushort value = ceil(counts[0] + 7 * counts[1] / 15 + counts[2] / 3);
    ushort sum   = g.count();

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
    ushort           g;            // steps from the root node
    ushort           h;            // heuristic value
    ushort           f;            // h + g
    grid_t           grid;         // grid of the current node

    // constructor for root node
    node(const grid_t &g_, const ushort h_):
        parent(nullptr),
        grid(g_),
        g(0),
        h(h_),
        f(h_)
    {}

    // constructor for non-root node
    node(const shared_ptr<node> &p, const action &a, const grid_t &g_,
         const ushort h_):
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

        for (ushort i = 0; i < size * size; ++i) {
            ushort temp;
            file >> temp;
            grid[i] = temp;
        }

        return *this;
    }

    void solve()
    {
        auto root = make_shared<node>(grid, heuristic(grid, size));

        priority_queue<shared_ptr<node>, vector<shared_ptr<node>>,
                       greater<shared_ptr<node>>>
            frontier;
        frontier.push(root);

        unordered_set<grid_t> explored = {grid};

        while (not frontier.empty()) {
            auto current = frontier.top();
            frontier.pop();

            if (current->h == 0) {
                set_solution(current);
                return;
            }

            for (const auto &[g, a] : get_childrens(current->grid)) {
                auto child =
                    make_shared<node>(current, a, g, heuristic(g, size));

                if (explored.find(g) == explored.end()) {  // not explored
                    explored.insert(child->grid);
                    frontier.push(child);
                }
            }
        }

        // This should never be reached
        return;
    }

    bool validate()
    {
        grid_t g           = grid;
        auto   take_action = [&g](const action &a, ushort size) {
            ushort x   = a.x * size;
            ushort y   = a.y;
            ushort x_1 = x + ((a.shape < 2) ? -size : size);
            ushort y_1 = y + ((a.shape == 1 or a.shape == 2) ? -1 : 1);

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

  protected:
    grid_t grid;

  private:
    ushort         size;
    vector<action> path;  // optimal path

    unordered_map<grid_t, action> get_childrens(grid_t &g)
    {
        ushort first_one_index;
        for (ushort i = 0; i < size * size; ++i) {
            if (g[i]) {
                first_one_index = i;
                break;
            }
        }

        ushort x = first_one_index / size;
        ushort y = first_one_index % size;

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

        unordered_map<grid_t, action> actions_to_grids;

        ushort valid_mask = 0x00f;  // One bit represents three actions

        // The invalid actions mask for special (x, y)
        ushort x_0_mask = 0x0003;  // 0011, [0 - 5]
        ushort x_n_mask = 0x000c;  // 1100, [6 - 11]
        ushort y_0_mask = 0x0009;  // 1001, [0 - 2, 9 - 11]
        ushort y_n_mask = 0x0006;  // 0110, [3 - 8]

        ushort invalid_mask = 0x0000;
        if (x == 0) {
            invalid_mask |= x_0_mask;
        } else if (x == size - 1) {
            invalid_mask |= x_n_mask;
        }
        if (y == 0) {
            invalid_mask |= y_0_mask;
        } else if (y == size - 1) {
            invalid_mask |= y_n_mask;
        }

        valid_mask &= ~invalid_mask;

        static int8_t pos_x_offset[] = {-1, 0, 0, 0, -1, 0, 1, 0, 0, 0, 0, 1};
        static int8_t pos_y_offset[] = {0, -1, 0, 0, 0, 1, 0, 1, 0, 0, -1, 0};
        static int8_t arm_x_offset[] = {1,  -1, -1, -1, 1, -1,
                                        -1, 1,  1,  1,  1, -1};
        static int8_t arm_y_offset[] = {-1, 1,  -1, 1,  1, -1,
                                        1,  -1, 1,  -1, 1, -1};
        static ushort shapes[]       = {2, 0, 1, 0, 3, 1, 0, 2, 3, 2, 3, 1};

        auto gen = [&actions_to_grids, &g, x, y, this](ushort i) {
            ushort pos_x = x + pos_x_offset[i];
            ushort pos_y = y + pos_y_offset[i];
            ushort arm_x = pos_x + arm_x_offset[i];
            ushort arm_y = pos_y + arm_y_offset[i];

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

        if (valid_mask & 0x0001) {
            gen(0);
            gen(1);
            gen(2);
        }

        if (valid_mask & 0x0002) {
            gen(3);
            gen(4);
            gen(5);
        }

        if (valid_mask & 0x0004) {
            gen(6);
            gen(7);
            gen(8);
        }

        if (valid_mask & 0x0008) {
            gen(9);
            gen(10);
            gen(11);
        }

        return actions_to_grids;
    }

    void set_solution(const shared_ptr<node> &goal)
    {
        path.resize(goal->g);
        ushort           index   = 0;
        shared_ptr<node> current = goal;
        // reconstruct the path, order does not matter
        while (current->parent != nullptr) {
            path[index++] = current->from_parent;
            current       = current->parent;
        }
    }
};

int main(int argc, char *argv[])
{
    for (int i = 0; i < 10; i++) {
        ifstream input(INPUTS[i]);

        solver s = solver().from_file(input);

        auto start = now();
        s.solve();
        auto end      = now();
        ns   duration = end - start;

        std::cout << "Time taken for " << INPUTS[i] << ": "
                  << duration.count() / 1e6 << " ms, "
                  << "solution valid: " << s.validate() << endl;

        ofstream output(OUTPUTS[i]);
        s.path_to(output);
    }

    return 0;
}
