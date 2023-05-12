#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>
#include <queue>
#include <set>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

using std::accumulate;
using std::endl;
using std::greater;
using std::ifstream;
using std::invalid_argument;
using std::istream;
using std::make_shared;
using std::max;
using std::min;
using std::ofstream;
using std::ostream;
using std::pair;
using std::priority_queue;
using std::reverse;
using std::set;
using std::shared_ptr;
using std::unordered_set;
using std::vector;

using width_t = unsigned short;
using grid_t  = vector<vector<bool>>;
using value_t = unsigned;

const char *inputs[] = {"../input/input0.txt", "../input/input1.txt",
                        "../input/input2.txt", "../input/input3.txt",
                        "../input/input4.txt", "../input/input5.txt",
                        "../input/input6.txt", "../input/input7.txt",
                        "../input/input8.txt", "../input/input9.txt"};

const char *outputs[] = {"../output/output0.txt", "../output/output1.txt",
                         "../output/output2.txt", "../output/output3.txt",
                         "../output/output4.txt", "../output/output5.txt",
                         "../output/output6.txt", "../output/output7.txt",
                         "../output/output8.txt", "../output/output9.txt"};

template<> class std::hash<grid_t>
{
  public:
    size_t operator()(const grid_t &grid) const
    {
        size_t hash = 0;
        for (auto &row : grid) {
            hash ^= std::hash<vector<bool>>()(row);
        }
        return hash;
    }
};

ostream &operator<<(ostream &os, const grid_t &grid)
{
    for (auto &row : grid) {
        for (const auto &cell : row) {
            os << cell << ' ';
        }
        os << endl;
    }
    return os;
}

class action
{
  public:
    width_t x;  // x coordinate of the centered cell
    width_t y;  // y coordinate of the centered cell
    /**
     * shape of the L-flip action, a value between 0 and 3
     * - 0: (x, y), (x - 1, y), (x, y + 1) | └-shape
     * - 1: (x, y), (x - 1, y), (x, y - 1) | ┘-shape
     * - 2: (x, y), (x + 1, y), (x, y - 1) | ┐-shape
     * - 3: (x, y), (x + 1, y), (x, y + 1) | ┌-shape
     */
    width_t shape;

    action(): x(0), y(0), shape(0) {}

    action(width_t x_, width_t y_, width_t shape_): x(x_), y(y_), shape(shape_)
    {}

    friend ostream &operator<<(ostream &os, const action &a)
    {
        os << a.x << ',' << a.y << ',' << a.shape;
        return os;
    }
};

class node
{
  public:
    shared_ptr<node> parent;       // parent node
    action           from_parent;  // action from parent
    grid_t           grid;         // lock grid
    value_t          g;            // steps from the root node
    value_t          h;            // heuristic value
    value_t          f;            // f = g + h

    // constructor for root node
    node(const grid_t &g_): parent(nullptr), grid(g_), g(0)
    {
        h = heuristic();
        f = h;
    }

    // constructor for non-root node
    node(const shared_ptr<node> &p, const action &a):
        parent(p),
        from_parent(a),
        g(p->g + 1)
    {
        int x_offset = (a.shape < 2) ? -1 : 1;
        int y_offset = (a.shape == 1 or a.shape == 2) ? -1 : 1;
        // copy grid and flip
        grid                      = p->grid;
        grid[a.x][a.y]            = !grid[a.x][a.y];
        grid[a.x + x_offset][a.y] = !grid[a.x + x_offset][a.y];
        grid[a.x][a.y + y_offset] = !grid[a.x][a.y + y_offset];

        h = heuristic();
        f = h + g;
    }

    bool is_effective_action(const action &a) const
    {
        int x_offset = (a.shape < 2) ? -1 : 1;
        int y_offset = (a.shape == 1 or a.shape == 2) ? -1 : 1;

        return grid[a.x][a.y] or grid[a.x + x_offset][a.y]
            or grid[a.x][a.y + y_offset];
    }

    friend ostream &operator<<(ostream &os, const node &n)
    {
        os << "grid:" << endl << n.grid;
        os << "g: " << n.g << endl;
        os << "h: " << n.h << endl;
        os << "f: " << n.f;
        return os;
    }

  private:
    value_t heuristic() const
    {
        value_t count = 0;
        for (auto &row : grid) {
            count = accumulate(row.begin(), row.end(), count);
        }

        switch (count) {
        case 2:
            return 2;
        case 1:
            return 3;
        case 0:
            return 0;
        default:
            return count / 3 + count % 3;
        }
    }
};

bool operator>(const shared_ptr<node> &lhs, const shared_ptr<node> &rhs)
{
    return lhs->f > rhs->f or (lhs->f == rhs->f and lhs->g > rhs->g);
}

bool operator<(const shared_ptr<node> &lhs, const shared_ptr<node> &rhs)
{
    return lhs->f < rhs->f or (lhs->f == rhs->f and lhs->g < rhs->g);
}

class sma_node: public node
{
  public:
    set<shared_ptr<node>>    children;
    vector<action>::iterator action_it;

    sma_node(const grid_t &g_, vector<action>::iterator &next):
        node(g_),
        action_it(next)
    {}

    sma_node(shared_ptr<sma_node> &p, const action &a,
             const vector<action>::iterator &it):
        node(p, *it),
        action_it(it + 1)
    {}
};

/* Priority queue with fixed capacity, used for SMA* */
/*
template<typename T, typename compare = std::less<T>> class fixed_size_pq
{
  public:
    fixed_size_pq(): max_size(0) {}

    fixed_size_pq(size_t max_size_): max_size(max_size_) {}

    optional<T> push(const T &item)
    {
        if (queue.size() < max_size) {
            queue.push_back(item);
            std::push_heap(queue.begin(), queue.end(), cmp);
            return std::nullopt;
        }

        auto min = std::min_element(queue.begin(), queue.end(), cmp);
        if (cmp(item, *min)) {
            return std::nullopt;
        } else {
            auto min_value = *min;
            *min           = item;
            std::make_heap(queue.begin(), queue.end(), cmp);
            return min_value;
        }
    }

    optional<T> pop()
    {
        if (queue.empty()) {
            return std::nullopt;
        }

        std::pop_heap(queue.begin(), queue.end(), cmp);
        auto back = queue.back();
        queue.pop_back();
        return back;
    }

    bool empty() const { return queue.empty(); }

    size_t size() const { return queue.size(); }

    const T &top() const { return queue.front(); }

  private:
    vector<T> queue;
    size_t    max_size;
    compare   cmp;
};
*/

enum method_t {
    A_star,
    IDA_star,
    RBFS,
    SMA_star,
};

class solver
{
  public:
    solver() {}

    solver from_file(istream &file)
    {
        file >> size;

        grid = grid_t(size, vector<bool>(size, false));
        for (auto &row : grid) {
            int bit;
            for (width_t i = 0; i < size; i++) {
                file >> bit;
                row[i] = bit;
            }
        }

        init_actions();

        max_depth = upper_bound();

        if (size < 2) {
            throw invalid_argument("The input grid is too small");
        }

        return *this;
    }

    solver set_type(method_t type_)
    {
        type = type_;
        return *this;
    }

    void solve()
    {
        switch (type) {
        case A_star:
            a_star();
            break;
        case IDA_star:
            ida_star();
            break;
        case RBFS:
            rbfs(make_shared<node>(grid), max_depth);
            ;
            break;
        case SMA_star:
            sma_star();
            break;
        }
    }

    void path_to(ostream &file)
    {
        file << path.size() << endl;
        for (auto &a : path) {
            file << a << endl;
        }
    }

    friend ostream &operator<<(ostream &os, const solver &s)
    {
        os << "size: " << s.size << endl;
        os << "grid: " << endl << s.grid;
        os << "max_depth: " << s.max_depth << endl;
        os << "inital heuristic: " << node(s.grid).h << endl;
        os << "type: ";
        switch (s.type) {
        case A_star:
            os << "A*";
            break;
        case IDA_star:
            os << "IDA*";
            break;
        case RBFS:
            os << "RBFS";
            break;
        case SMA_star:
            os << "SMA*";
            break;
        }
        return os;
    }

  private:
    method_t type = A_star;
    width_t  size;
    grid_t   grid;
    // valid actions in the grid
    vector<action> actions;
    // steps to take in a semi-optimal heuristic solution, used for IDA*
    value_t max_depth;
    // optimal path
    vector<action> path;

    // initialize actions
    void init_actions()
    {
        // The number of actions is calculated in the following way:
        // - cells in the corner: 4 * 1 = 4
        // - cells in the edge:   4 * (size - 2) * 2 = 8 * size - 16
        // - cells in the center: (size - 2)^2 * 4 = 4 * size^2 - 16 * size + 16
        // total: 4 - 8 * size + 4 * size^2
        actions.resize(4 - 8 * size + 4 * size * size);
        size_t index = 0;

        // 4 cells in the corner
        actions[index++] = action(0, 0, 3);
        actions[index++] = action(0, size - 1, 2);
        actions[index++] = action(size - 1, 0, 0);
        actions[index++] = action(size - 1, size - 1, 1);
        // cells in the edge
        /// up: x = 0
        for (width_t y = 1; y < size - 1; y++) {
            actions[index++] = action(0, y, 2);
            actions[index++] = action(0, y, 3);
        }
        /// bottom: x = size - 1
        for (width_t y = 1; y < size - 1; y++) {
            actions[index++] = action(size - 1, y, 0);
            actions[index++] = action(size - 1, y, 1);
        }
        /// left: y = 0
        for (width_t x = 1; x < size - 1; x++) {
            actions[index++] = action(x, 0, 0);
            actions[index++] = action(x, 0, 3);
        }
        /// right: y = size - 1
        for (width_t x = 1; x < size - 1; x++) {
            actions[index++] = action(x, size - 1, 1);
            actions[index++] = action(x, size - 1, 2);
        }
        // cells in the center
        for (width_t x = 1; x < size - 1; x++) {
            for (width_t y = 1; y < size - 1; y++) {
                actions[index++] = action(x, y, 0);
                actions[index++] = action(x, y, 1);
                actions[index++] = action(x, y, 2);
                actions[index++] = action(x, y, 3);
            }
        }
    }

    // get the number of steps in a semi-optimal heuristic solution
    value_t upper_bound()
    {
        if (size == 2) {
            return 4;
        }

        value_t steps = 0;
        grid_t  copy  = grid;

        // First, we try to eliminate all L-shaped cells.
        for (width_t i = 0; i < size - 1; i++) {
            for (width_t j = 0; j < size - 1; j++) {
                value_t count = copy[i][j] + copy[i][j + 1] + copy[i + 1][j]
                              + copy[i + 1][j + 1];
                if (count == 3) {
                    steps++;
                    copy[i][j]         = false;
                    copy[i][j + 1]     = false;
                    copy[i + 1][j]     = false;
                    copy[i + 1][j + 1] = false;
                } else if (count == 4) {
                    steps++;
                    copy[i][j]     = false;
                    copy[i][j + 1] = false;
                    copy[i + 1][j] = false;
                }
            }
        }

        // Second, we use a moving window to eliminate all 3x3-1 squares. The
        // 3x3-1 square is a 3x3 square without a corner. Some of the cells
        // won't be dealed.
        /// The max steps to eliminate all 1s in the window, indexed by count
        /// of 1s in the window.
        value_t step_in_3x3_window[] = {0, 3, 2, 3, 4, 3};
        /// Iterate over the grid.
        for (width_t i = 0; i < size - 2; i++) {
            for (width_t j = 0; j < size - 2; j++) {
                if (copy[i][j]) {
                    value_t count = copy[i + 0][j + 0] + copy[i + 0][j + 1]
                                  + copy[i + 0][j + 2] + copy[i + 1][j + 0]
                                  + copy[i + 1][j + 1] + copy[i + 1][j + 2]
                                  + copy[i + 2][j + 0] + copy[i + 2][j + 1];
                    copy[i + 0][j + 0] = copy[i + 0][j + 1] = false;
                    copy[i + 0][j + 2] = copy[i + 1][j + 0] = false;
                    copy[i + 1][j + 1] = copy[i + 1][j + 2] = false;
                    copy[i + 2][j + 0] = copy[i + 2][j + 1] = false;
                    steps += step_in_3x3_window[count];
                }
            }
        }
        // Third, handle the un-dealed cells. We use a 3x2 window instead.
        /// Deal with the bottom-right 2x2 window first
        value_t step_in_2x2_window[] = {0, 3, 2};
        value_t count = copy[size - 1][size - 1] + copy[size - 1][size - 2]
                      + copy[size - 2][size - 1] + copy[size - 2][size - 2];
        copy[size - 1][size - 1] = copy[size - 1][size - 2] = false;
        copy[size - 2][size - 1] = copy[size - 2][size - 2] = false;
        steps += step_in_2x2_window[count];
        /// Shift the 3x2 window on two edges
        for (width_t i = 0; i < size - 2; i++) {
            // Right edge
            if (copy[i][size - 1] or copy[i][size - 2]) {
                value_t count = copy[i + 0][size - 1] + copy[i + 0][size - 2]
                              + copy[i + 1][size - 1] + copy[i + 1][size - 2]
                              + copy[i + 2][size - 1] + copy[i + 2][size - 2];
                copy[i + 0][size - 1] = copy[i + 0][size - 2] = false;
                copy[i + 1][size - 1] = copy[i + 1][size - 2] = false;
                copy[i + 2][size - 1] = copy[i + 2][size - 2] = false;
                steps += step_in_3x3_window[count];
            }
            // Bottom edge
            if (copy[size - 1][i] or copy[size - 2][i]) {
                value_t count = copy[size - 1][i + 0] + copy[size - 1][i + 1]
                              + copy[size - 1][i + 2] + copy[size - 2][i + 0]
                              + copy[size - 2][i + 1] + copy[size - 2][i + 2];
                copy[size - 1][i + 0] = copy[size - 1][i + 1] = false;
                copy[size - 1][i + 2] = copy[size - 2][i + 0] = false;
                copy[size - 2][i + 1] = copy[size - 2][i + 2] = false;
                steps += step_in_3x3_window[count];
            }
        }

        return steps;
    }

    void set_solution(const shared_ptr<node> &goal)
    {
        shared_ptr<node> current = goal;
        // reconstruct the path
        while (current->parent != nullptr) {
            path.push_back(current->from_parent);
            current = current->parent;
        }

        reverse(path.begin(), path.end());
    }

    void a_star()
    {
        auto root = make_shared<node>(grid);

        priority_queue<shared_ptr<node>, vector<shared_ptr<node>>,
                       greater<shared_ptr<node>>>
            frontier;
        frontier.push(root);

        unordered_set<grid_t> explored;

        while (not frontier.empty()) {
            auto current = frontier.top();
            frontier.pop();

            if (current->h == 0) {
                set_solution(current);
                return;
            }

            for (const auto &a : actions) {
                if (current->is_effective_action(a)) {
                    auto child = make_shared<node>(current, a);

                    if (child->f > max_depth) {
                        continue;
                    }

                    if (explored.find(child->grid) != explored.end()) {
                        continue;
                    }

                    explored.emplace(child->grid);
                    frontier.push(child);
                }
            }
        }

        // This should never be reached
        return;
    }

    void ida_star()
    {
        shared_ptr<node> root = make_shared<node>(grid);

        value_t limit = root->h;

        while (limit <= max_depth) {
            value_t next_limit = max_depth;

            priority_queue<shared_ptr<node>, vector<shared_ptr<node>>,
                           greater<shared_ptr<node>>>
                frontier;
            frontier.push(root);

            unordered_set<grid_t> explored;
            while (not frontier.empty()) {
                auto current = frontier.top();
                frontier.pop();

                if (current->h == 0) {
                    set_solution(current);
                    return;
                }

                for (const auto &a : actions) {
                    if (current->is_effective_action(a)) {
                        auto child = make_shared<node>(current, a);
                        if (child->f > limit) {
                            next_limit = min(next_limit, child->f);
                            continue;
                        }

                        if (explored.find(child->grid) != explored.end()) {
                            continue;
                        }

                        explored.insert(child->grid);
                        frontier.push(child);
                    }
                }
            }
            limit = next_limit;
        }

        // This should never be reached
        return;
    }

    pair<bool, value_t> rbfs(const shared_ptr<node> &n, value_t limit)
    {
        if (n->h == 0) {
            set_solution(n);
            return {true, n->f};
        }

        vector<shared_ptr<node>> successors;
        for (const auto &a : actions) {
            if (n->is_effective_action(a)) {
                auto child = make_shared<node>(n, a);
                successors.push_back(child);
            }
        }

        if (successors.empty()) {
            return {false, max_depth};
        }

        for (auto &s : successors) {
            s->f = max(s->f, n->f);
        }

        while (true) {
            shared_ptr<node> best        = successors[0];
            shared_ptr<node> alternative = successors[1];
            if (best > alternative) {
                swap(best, alternative);
            }
            for (auto &s : successors) {
                if (s < best) {
                    alternative = best;
                    best        = s;
                } else if (s < alternative) {
                    alternative = s;
                }
            }
            if (best->f > limit) {
                return {false, best->f};
            }
            auto result = rbfs(best, min(limit, alternative->f));
            best->f     = result.second;
            if (result.first) {
                return {true, best->f};
            }
        }
    }

    void sma_star() {}

    // Caution: remember to push the child to `p`
    shared_ptr<sma_node> sma_next_child(shared_ptr<sma_node> &p)
    {
        for (; p->action_it != actions.end(); p->action_it++) {
            if (p->is_effective_action(*(p->action_it))) {
                auto child =
                    make_shared<sma_node>(p, *(p->action_it), actions.begin());
                p->action_it++;
                return child;
            }
        }
        return nullptr;
    }
};

int main(int argc, char *argv[])
{
    for (int i = 0; i < 6; i++) {
        ifstream input(inputs[i]);

        solver s = solver().from_file(input).set_type(A_star);
        s.solve();

        ofstream output(outputs[i]);
        s.path_to(output);
    }

    return 0;
}
