#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <stdexcept>
#include <unordered_set>
#include <utility>
#include <vector>

using std::endl;
using std::greater;
using std::ifstream;
using std::invalid_argument;
using std::istream;
using std::make_shared;
using std::min;
using std::ofstream;
using std::ostream;
using std::priority_queue;
using std::reverse;
using std::shared_ptr;
using std::unordered_set;
using std::vector;

using width_t = unsigned short;
using grid_t  = vector<vector<bool>>;
using value_t = unsigned short;

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

const int bound[] = {5, 4, 5, 7, 7, 7, 11, 14, 16, 23};

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

bool operator==(const action &a1, const action &a2)
{
    return a1.x == a2.x and a1.y == a2.y and a1.shape == a2.shape;
}

bool is_effective_action(const grid_t &g, const action &a)
{
    int x_1 = a.x + ((a.shape < 2) ? -1 : 1);
    int y_1 = a.y + ((a.shape == 1 or a.shape == 2) ? -1 : 1);

    return (g[a.x][a.y] or g[x_1][a.y] or g[a.x][y_1]);
}

void take_action(grid_t &g, const action &a)
{
    int x_1 = a.x + ((a.shape < 2) ? -1 : 1);
    int y_1 = a.y + ((a.shape == 1 or a.shape == 2) ? -1 : 1);

    g[a.x][a.y] = !g[a.x][a.y];
    g[x_1][a.y] = !g[x_1][a.y];
    g[a.x][y_1] = !g[a.x][y_1];
}

value_t heuristic(const grid_t &g)
{
    /* TODO: This function is not consistent */
    width_t                  size = g.size();
    vector<vector<unsigned>> dist(size, vector<unsigned>(size, 0));
    /* Iterate over the grid to find all 1s with other 1s surrounded and
     * mark it */
    for (width_t i = 0; i < size - 1; ++i) {
        for (width_t j = 0; j < size - 1; ++j) {
            unsigned count =
                g[i][j] + g[i + 1][j] + g[i][j + 1] + g[i + 1][j + 1];
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
    /* Count the number of 1s, 2s and 3s */
    float counts[3] = {0, 0, 0};
    for (width_t i = 0; i < size; ++i) {
        for (width_t j = 0; j < size; ++j) {
            if (g[i][j] and dist[i][j])
                ++counts[dist[i][j] - 1];
            else if (g[i][j])
                ++counts[0];
        }
    }

    return ceil(counts[0] + counts[1] / 2 + counts[2] / 3);
}

class node
{
  public:
    shared_ptr<node> parent;       // parent node
    action           from_parent;  // action from parent
    value_t          g;            // steps from the root node
    value_t          h;            // heuristic value
    value_t          f;            // f = g + h

    // constructor for root node
    node(const grid_t &g_, const value_t h_):
        parent(nullptr),
        g(0),
        h(h_),
        f(h_)
    {}

    // constructor for non-root node
    node(const shared_ptr<node> &p, const action &a, const value_t h_):
        parent(p),
        from_parent(a),
        g(p->g + 1),
        h(h_),
        f(p->g + 1 + h)
    {}

    grid_t get_grid(const grid_t &root) const
    {
        if (parent == nullptr) {
            return root;
        }

        grid_t g = root;

        take_action(g, from_parent);
        auto p = parent;

        while (p->parent != nullptr) {
            take_action(g, p->from_parent);
            p = p->parent;
        }

        return g;
    }

    friend ostream &operator<<(ostream &os, const node &n)
    {
        os << "g: " << n.g << endl;
        os << "h: " << n.h << endl;
        os << "f: " << n.f;
        return os;
    }
};

bool operator>(const shared_ptr<node> &lhs, const shared_ptr<node> &rhs)
{
    return lhs->f > rhs->f or (lhs->f == rhs->f and lhs->g < rhs->g);
}

bool operator<(const shared_ptr<node> &lhs, const shared_ptr<node> &rhs)
{
    return lhs->f < rhs->f or (lhs->f == rhs->f and lhs->g > rhs->g);
}

/*
class sma_node: public node
{
  public:
    shared_ptr<sma_node>      parent;
    set<shared_ptr<sma_node>> children;
    vector<action>::iterator  action_it;
    queue<action>             removed;

    sma_node(const grid_t &g_, vector<action>::iterator next):
        node(g_),
        action_it(next)
    {}

    sma_node(shared_ptr<sma_node> &p, const action &a,
             const vector<action>::iterator &it):
        node(p, *it),
        action_it(it + 1)
    {}
};
*/

/* Priority queue with fixed capacity, used for SMA* */
/*
template<typename T, typename compare = std::greater<T>> class fixed_size_pq
{
  public:
    fixed_size_pq(): max_size(0) {}

    fixed_size_pq(size_t max_size_): max_size(max_size_) {}

    bool push(const T &item)
    {
        if (full()) {
            return false;
        }
        queue.push_back(item);
        std::push_heap(queue.begin(), queue.end(), cmp);
        return true;
    }

    bool pop()
    {
        if (queue.empty()) {
            return false;
        }

        std::pop_heap(queue.begin(), queue.end(), cmp);
        auto back = queue.back();
        queue.pop_back();
        return true;
    }

    T pop_max()
    {
        auto min   = std::min_element(queue.begin(), queue.end(), cmp);
        auto index = std::distance(queue.begin(), min);
        auto item  = *min;
        std::swap(queue[index], queue.back());
        queue.pop_back();
        while (index > 0) {
            auto parent = (index - 1) / 2;
            if (!cmp(queue[index], queue[parent])) {
                std::swap(queue[index], queue[parent]);
                index = parent;
            } else {
                break;
            }
        }
        return item;
    }

    bool have(const T &item) const
    {
        return std::find(queue.begin(), queue.end(), item) != queue.end();
    }

    bool have_all(const set<T> &s) const
    {
        size_t count = 0;
        for (auto &item : queue) {
            if (s.find(item) != s.end()) {
                count++;
            }
        }
        return count == s.size();
    }

    void refresh() { std::make_heap(queue.begin(), queue.end(), cmp); }

    bool full() const { return queue.size() == max_size; }

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

        max_children = 0xFFFF;

        if (size < 2) {
            throw invalid_argument("The input grid is too small");
        }

        return *this;
    }

    solver set_bound()
    {
        max_depth = upper_bound();
        return *this;
    }

    solver set_bound(int bound_)
    {
        max_depth = bound_;
        return *this;
    }

    solver set_type(method_t type_)
    {
        type = type_;
        return *this;
    }

    solver set_expand_num(size_t num)
    {
        max_children = num;
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
            /* rbfs(make_shared<node>(grid), max_depth); */
            break;
        case SMA_star:
            /* sma_star(); */
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
        os << "inital heuristic: " << heuristic(s.grid) << endl;
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

  protected:
    grid_t grid;

  private:
    method_t type = A_star;
    width_t  size;
    // valid actions in the grid
    vector<action> actions;
    // max number of children to expand in a node
    size_t max_children = 4;
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
                    unsigned count = copy[i + 0][j + 0] + copy[i + 0][j + 1]
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
        unsigned step_in_2x2_window[] = {0, 3, 2};
        unsigned count = copy[size - 1][size - 1] + copy[size - 1][size - 2]
                       + copy[size - 2][size - 1] + copy[size - 2][size - 2];
        copy[size - 1][size - 1] = copy[size - 1][size - 2] = false;
        copy[size - 2][size - 1] = copy[size - 2][size - 2] = false;
        steps += step_in_2x2_window[count];
        /// Shift the 3x2 window on two edges
        for (width_t i = 0; i < size - 2; i++) {
            // Right edge
            if (copy[i][size - 1] or copy[i][size - 2]) {
                unsigned count = copy[i + 0][size - 1] + copy[i + 0][size - 2]
                               + copy[i + 1][size - 1] + copy[i + 1][size - 2]
                               + copy[i + 2][size - 1] + copy[i + 2][size - 2];
                copy[i + 0][size - 1] = copy[i + 0][size - 2] = false;
                copy[i + 1][size - 1] = copy[i + 1][size - 2] = false;
                copy[i + 2][size - 1] = copy[i + 2][size - 2] = false;
                steps += step_in_3x3_window[count];
            }
            // Bottom edge
            if (copy[size - 1][i] or copy[size - 2][i]) {
                unsigned count = copy[size - 1][i + 0] + copy[size - 1][i + 1]
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
        auto root = make_shared<node>(grid, heuristic(grid));

        priority_queue<shared_ptr<node>, vector<shared_ptr<node>>,
                       greater<shared_ptr<node>>>
            frontier;
        frontier.push(root);

        unordered_set<grid_t> explored = {grid};

        while (not frontier.empty()) {
            auto current = frontier.top();
            frontier.pop();

            /* std::cout << "Frontier size: " << frontier.size() */
            /*           << ", Node visited: " << ++num_visited */
            /*           << ", Tree size: " << explored.size() */
            /*           << ", Current f: " << current->f << ", h: " <<
             * current->h */
            /*           << ", g: " << current->g << std::endl; */

            if (current->h == 0) {
                std::cout << "Solution found!" << std::endl;
                set_solution(current);
                return;
            }

            grid_t                   current_grid = current->get_grid(grid);
            vector<shared_ptr<node>> children;
            for (const auto &a : actions) {
                if (is_effective_action(current_grid, a)) {
                    auto child_grid = current_grid;
                    take_action(child_grid, a);
                    auto child =
                        make_shared<node>(current, a, heuristic(child_grid));

                    if (child->f > max_depth) {
                        continue;
                    }

                    if (explored.find(child_grid) != explored.end()) {
                        continue;
                    }

                    children.push_back(child);
                }
            }

            std::sort(children.begin(), children.end());
            size_t index = 0;
            for (const auto &child : children) {
                auto child_grid = current_grid;
                take_action(child_grid, child->from_parent);
                explored.insert(child_grid);
                frontier.push(child);
                if (++index == max_children) {
                    break;
                }
            }
        }

        // This should never be reached
        return;
    }

    void ida_star()
    {
        shared_ptr<node> root = make_shared<node>(grid, heuristic(grid));

        value_t limit = root->h;

        while (true) {
            value_t next_limit = 0xFFFF;

            priority_queue<shared_ptr<node>, vector<shared_ptr<node>>,
                           greater<shared_ptr<node>>>
                frontier;
            frontier.push(root);

            unordered_set<grid_t> explored = {grid};

            size_t num_visited = 0;

            while (not frontier.empty()) {
                auto current = frontier.top();
                frontier.pop();

                std::cout << "Limit: " << limit
                          << ", Frontier size: " << frontier.size()
                          << ", Node visited: " << ++num_visited
                          << ", Tree size: " << explored.size()
                          << ", Current f: " << current->f
                          << ", h: " << current->h << ", g: " << current->g
                          << std::endl;

                if (current->h == 0) {
                    set_solution(current);
                    return;
                }

                grid_t current_grid = current->get_grid(grid);
                for (const auto &a : actions) {
                    if (is_effective_action(current_grid, a)) {
                        auto child_grid = current_grid;
                        take_action(child_grid, a);

                        auto child = make_shared<node>(current, a,
                                                       heuristic(child_grid));

                        if (child->f > limit) {
                            next_limit = min(child->f, next_limit);
                            continue;
                        }

                        if (explored.find(child_grid) != explored.end()) {
                            continue;
                        }

                        explored.insert(child_grid);
                        frontier.push(child);
                    }
                }
            }
            limit = next_limit;
        }

        // This should never be reached
        return;
    }

    /*
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
    */

    /*
    void sma_star()
    {
        auto root     = make_shared<sma_node>(grid, actions.begin());
        auto frontier = fixed_size_pq<shared_ptr<sma_node>>(SMA_QUEUE_SIZE);
        frontier.push(root);
        while (!frontier.empty()) {
            auto current = frontier.top();
            frontier.pop();

            if (current->h == 0) {
                set_solution(current);
                return;
            }

            auto child = next_child(current);
            while (child != nullptr
                   and (child->f > max_depth or child->duplicate()))
            {
                child = next_child(current);
            }
            if (child == nullptr) {
                value_t min_f = max_depth;
                for (const auto &c : current->children) {
                    min_f = min(min_f, c->f);
                }
                if (min_f > current->f) {
                    current->f  = min_f;
                    auto parent = current->parent;
                    while (parent != nullptr) {
                        if (parent->action_it != actions.end()
                            or parent->f >= min_f)
                        {
                            break;
                        }
                        parent->f = current->f;
                        parent    = parent->parent;
                    }
                    frontier.refresh();
                }
                if (!frontier.have_all(current->children)) {
                    frontier.push(current);
                }
                continue;
            }
            current->children.emplace(child);
            child->f = max(current->f, child->f);
            while (frontier.full()) {
                auto max_node = frontier.pop_max();
                auto parent   = max_node->parent;
                parent->children.erase(max_node);
                parent->removed.emplace(max_node->from_parent);
                if (!frontier.have(parent)) {
                    frontier.push(parent);
                }
            }
            frontier.push(child);
        }
    }

    // Caution: remember to push the child to `p`
    shared_ptr<sma_node> next_child(shared_ptr<sma_node> &p)
    {
        for (; p->action_it != actions.end(); p->action_it++) {
            if (p->is_effective_action(*(p->action_it))) {
                auto child =
                    make_shared<sma_node>(p, *(p->action_it), actions.begin());
                p->action_it++;
                return child;
            }
        }
        if (p->removed.empty())
            return nullptr;
        auto child = make_shared<sma_node>(p, p->removed.back(), actions.end());
        p->removed.pop();
        return child;
    }
    */
};

int main(int argc, char *argv[])
{
    for (int i = 9; i < 10; i++) {
        ifstream input(inputs[i]);

        solver s = solver()
                       .from_file(input)
                       .set_bound(bound[i])
                       .set_type(A_star)
                       .set_expand_num(2);

        if (i == 8) {
            s.set_expand_num(3);
        }

        s.solve();

        ofstream output(outputs[i]);
        s.path_to(output);
    }

    return 0;
}
