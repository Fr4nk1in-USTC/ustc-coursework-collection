#include "config.h"
#include "graph.h"

#include <fstream>
#include <random>
#include <set>

using std::endl;
using std::mt19937;
using std::ofstream;
using std::random_device;
using std::set;
using std::uniform_int_distribution;
using std::vector;

const int min = -10;
const int max = 50;

void break_neg_cycle(Graph &g)
{
    // Add a virtual vertex
    Vertex v;
    g.vertices.push_back(v);
    int n = g.size();
    for (int i = 0; i < n - 1; i++) {
        g.add_edge(n - 1, i, 0);
    }
    // Bellman-Ford, but when find a negative cycle, break it
    vector<int> dist(n, inf);
    vector<int> prev(n, -1);

    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n; j++) {
            Edge *p = g.vertices[j].head;
            while (p != nullptr) {
                if (dist[p->dst] > dist[j] + p->weight) {
                    dist[p->dst] = dist[j] + p->weight;
                    prev[p->dst] = j;
                }
                p = p->next;
            }
        }
    }

    for (int i = 0; i < n; i++) {
        Edge *p = g.vertices[i].head;
        while (p != nullptr) {
            if (dist[p->dst] > dist[i] + p->weight) {
                // Find the negative cycle
                int u = i;  // A vertex in the negative cycle
                int v;      // prev[v] = u, (u, v) is the edge to remove

                set<int> cycle;
                while (cycle.find(u) == cycle.end()) {
                    cycle.insert(u);
                    v = u;
                    u = prev[u];
                }
                g.remove_edge(u, v);
                g.vertices.pop_back();
                break_neg_cycle(g);
                return;
            }
            p = p->next;
        }
    }

    // Remove the virtual vertex
    g.vertices.pop_back();
}

void graph_to_file(Graph &g, const char *filename)
{
    ofstream fout(filename);
    for (int i = 0; i < g.size(); i++) {
        Edge *p = g.vertices[i].head;
        while (p != nullptr) {
            fout << i << " " << p->dst << " " << p->weight << endl;
            p = p->next;
        }
    }
    fout.close();
}

int main()
{
    random_device                 rd;
    mt19937                       gen(rd());
    uniform_int_distribution<int> w_dis(min, max);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            int   n = num_vertices[i];
            int   e = num_edges[i][j];
            Graph g(n);

            uniform_int_distribution<int> v_dis(0, n - 1);

            for (int u = 0; u < n; u++) {
                set<int> s;
                s.insert(u);
                while (s.size() <= (std::size_t)e) {
                    int v;
                    do {
                        v = v_dis(gen);
                    } while (s.find(v) != s.end());
                    s.insert(v);
                    g.add_edge(u, v, w_dis(gen));
                }
            }

            break_neg_cycle(g);
            graph_to_file(g, input_files[i][j]);
        }
    }
}
