#include <algorithm>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <ostream>
#include <queue>
#include <tuple>
#include <vector>

using std::cin;
using std::copy;
using std::cout;
using std::endl;
using std::greater;
using std::map;
using std::ostream;
using std::priority_queue;
using std::swap;
using std::vector;

class puzzle
{
  public:
    int grid[9];
    int pos[9];

    puzzle(int _grid[9])
    {
        for (int i = 0; i < 9; i++) {
            grid[i]       = _grid[i];
            pos[_grid[i]] = i;
        }
    }

    puzzle(const puzzle &other)
    {
        copy(other.grid, other.grid + 9, grid);
        copy(other.pos, other.pos + 9, pos);
    }

    ~puzzle() {};

    operator int() const
    {
        int res = 0;
        for (int i = 0; i < 9; i++) {
            res *= 10;
            res += grid[i];
        }
        return res;
    }
};

ostream &operator<<(ostream &os, const puzzle &p)
{
    char convert[] = " 12345678";
    os << "┌───┬───┬───┐" << endl
       << "│ " << convert[p.grid[0]] << " │ " << convert[p.grid[1]] << " │ "
       << convert[p.grid[2]] << " │" << endl
       << "├───┼───┼───┤" << endl
       << "│ " << convert[p.grid[3]] << " │ " << convert[p.grid[4]] << " │ "
       << convert[p.grid[5]] << " │" << endl
       << "├───┼───┼───┤" << endl
       << "│ " << convert[p.grid[6]] << " │ " << convert[p.grid[7]] << " │ "
       << convert[p.grid[8]] << " │" << endl
       << "└───┴───┴───┘" << endl;
    return os;
}

typedef struct state {
    puzzle p;
    int    f;
    int    g;
    int    h;
} state;

bool operator>(const state &l, const state &r)
{
    return l.f > r.f;
}

bool operator<(const state &l, const state &r)
{
    return l.f < r.f;
}

bool operator==(const state &l, const state &r)
{
    return l.f == r.f;
}

int h1(const puzzle &p)
{
    int distance = 0;
    for (int i = 1; i < 9; i++) {
        distance += p.pos[i] != i;
    }
    return distance;
}

int h2(const puzzle &p)
{
    int distance = 0;
    for (int i = 1; i < 9; i++) {
        distance += abs(p.pos[i] / 3 - i / 3) + abs(p.pos[i] % 3 - i % 3);
    }
    return distance;
}

int h3(const puzzle &p)
{
    return h1(p) + h2(p);
}

vector<state> expand(const state &s, int h(const puzzle &))
{
    int new_h    = 0;
    int new_g    = s.g + 1;
    int zero_pos = s.p.pos[0];

    vector<state> states;
    // Up
    if (zero_pos > 2) {
        puzzle up(s.p);
        swap(up.grid[zero_pos], up.grid[zero_pos - 3]);
        swap(up.pos[0], up.pos[up.grid[zero_pos]]);
        new_h = h(up);
        states.push_back({up, new_g + new_h, new_g, new_h});
    }
    // Down
    if (zero_pos < 6) {
        puzzle down(s.p);
        swap(down.grid[zero_pos], down.grid[zero_pos + 3]);
        swap(down.pos[0], down.pos[down.grid[zero_pos]]);
        new_h = h(down);
        states.push_back({down, new_g + new_h, new_g, new_h});
    }

    // Left
    if (zero_pos % 3 > 0) {
        puzzle left(s.p);
        swap(left.grid[zero_pos], left.grid[zero_pos - 1]);
        swap(left.pos[0], left.pos[left.grid[zero_pos]]);
        new_h = h(left);
        states.push_back({left, new_g + new_h, new_g, new_h});
    }

    // Right
    if (zero_pos % 3 < 2) {
        puzzle right(s.p);
        swap(right.grid[zero_pos], right.grid[zero_pos + 1]);
        swap(right.pos[0], right.pos[right.grid[zero_pos]]);
        new_h = h(right);
        states.push_back({right, new_g + new_h, new_g, new_h});
    }

    return states;
}

int solve(puzzle p, int h(const puzzle &))
{
    map<int, int>                                        visited;
    priority_queue<state, vector<state>, greater<state>> frontier;
    frontier.push({p, h(p), 0, h(p)});

    while (!frontier.empty()) {
        state s = frontier.top();
        frontier.pop();
        visited[s.p] = s.g;

        if (s.h == 0) {
            /* cout << s.p << endl; */
            /* cout << "f = " << s.f << endl; */
            /* cout << "g = " << s.g << endl; */
            /* cout << "h = " << s.h << endl; */
            return s.f;
        }

        vector<state> next_states = expand(s, h);
        for (auto &next_state : next_states) {
            if (visited.find(next_state.p) != visited.end()
                && visited[next_state.p] <= next_state.g)
            {
                continue;
            }
            frontier.push(next_state);
        }
    }
    return -1;
}

void help_info()
{
    cout << "A simple program to solve the 8-puzzle problem using A* algorithm "
            "with 3 "
         << endl
         << "different heuristic functions. The heuri-stic functions are:"
         << endl
         << "  h1: the number of tiles that are not in their goal position."
         << endl
         << "  h2: the sum of the distances of the tiles from their goal "
            "positions."
         << endl
         << "  h3: the sum of h1 and h2." << endl
         << "Note that h1 and h2 are admissible heuristic functions which "
            "guarantee that A* "
         << endl
         << "algorithm will find the optimal solution. But h3 is not "
            "admissible, which means "
         << endl
         << "that the A* algorithm may find the suboptimal solution. A example "
            "puzzle which "
         << endl
         << "will illustrate the suboptimal solution is:" << endl
         << "    ┌───┬───┬───┐" << endl
         << "    │ 2 │ 7 │ 8 │" << endl
         << "    ├───┼───┼───┤" << endl
         << "    │ 6 │ 5 │ 4 │" << endl
         << "    ├───┼───┼───┤" << endl
         << "    │ 1 │   │ 3 │" << endl
         << "    └───┴───┴───┘" << endl;
}

void usage(char *prog_name)
{
    cout << "usage:" << prog_name << " [options] [puzzle]" << endl
         << "  " << prog_name << " puzzle" << endl
         << "    solve the provided puzzle and show differences between "
            "heuristic functions."
         << endl
         << "    The puzzle is a 9-digit number, where 0 represents the blank "
            "tile. For "
         << endl
         << "    example, the puzzle above can be provided as: 278654103"
         << endl
         << "  " << prog_name << " -e, --example" << endl
         << "    solve the example puzzle and show differences between "
            "heuristic functions."
         << endl
         << "  " << prog_name << " -h, --help" << endl
         << "    show this help message" << endl
         << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        usage(argv[0]);
        return -1;
    }

    int init_grid[9] = {2, 7, 8, 6, 5, 4, 1, 0, 3};

    if (strcmp(argv[1], "-h") == 0 or strcmp(argv[1], "--help") == 0) {
        help_info();
        usage(argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "-e") != 0 and strcmp(argv[1], "--example") != 0) {
        if (strlen(argv[1]) != 9) {
            cout << "Invalid puzzle! See the usages of the program below!"
                 << endl
                 << endl;
            usage(argv[0]);
            return -1;
        }
        int count[9] = {0};
        for (int i = 0; i < 9; i++) {
            init_grid[i] = argv[1][i] - '0';
            if (init_grid[i] < 0 or init_grid[i] > 8) {
                cout << "Invalid puzzle! See the usages of the program below!"
                     << endl
                     << endl;
                usage(argv[0]);
                return -1;
            }
            count[init_grid[i]]++;
        }
        for (int i = 0; i < 9; i++) {
            if (count[i] != 1) {
                cout << "Invalid puzzle! See the usages of the program below!"
                     << endl
                     << endl;
                usage(argv[0]);
                return -1;
            }
        }
    }

    puzzle init_puzzle(init_grid);

    cout << "input puzzle: " << endl
         << init_puzzle << endl
         << "heuristics:" << endl
         << "    h1 (mismatched count):   " << h1(init_puzzle) << endl
         << "    h2 (manhattan distance): " << h2(init_puzzle) << endl
         << "    h3 (h1 + h2):            " << h3(init_puzzle) << endl;

    if (solve(init_puzzle, h2) == -1) {
        cout << "The puzzle is unsolvable!" << endl;
        return -1;
    }

    cout << "step of solution found by A*:" << endl
         << "    using h1: " << solve(init_puzzle, h1) << endl
         << "    using h2: " << solve(init_puzzle, h2) << endl
         << "    using h3: " << solve(init_puzzle, h3) << endl;
    return 0;
}
