#include "config.h"
#include "graph.h"
#include "min-priority-queue.h"

#include <chrono>
#include <fstream>
#include <stdexcept>
#include <vector>

using std::endl;
using std::ifstream;
using std::ofstream;
using std::vector;
using ns = std::chrono::nanoseconds;
auto now = std::chrono::high_resolution_clock::now;

bool bellman_ford(Graph &g, int src, vector<int> &dist, vector<int> &prev)
{
    int n = g.size();
    dist.resize(n, inf);
    prev.resize(n, -1);
    dist[src] = 0;

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
                return false;
            }
            p = p->next;
        }
    }
    return true;
}

void dijkstra(Graph &g, int src, vector<int> &dist, vector<int> &prev)
{
    int n = g.size();
    dist.assign(n, inf);
    prev.assign(n, -1);
    dist[src] = 0;

    MinPriorityQueue q(n);
    q.decrease_key(src, 0);

    while (!q.empty()) {
        int   u = q.extract_min();
        Edge *p = g.vertices[u].head;
        while (p != nullptr) {
            if (dist[p->dst] > dist[u] + p->weight) {
                dist[p->dst] = dist[u] + p->weight;
                prev[p->dst] = u;
                q.decrease_key(p->dst, dist[p->dst]);
            }
            p = p->next;
        }
    }
}

void johnson(Graph &g, vector<vector<int>> &dist, vector<vector<int>> &prev)
{
    int n = g.size();
    // Add a virtual vertex
    Vertex v;
    g.vertices.push_back(v);
    for (int i = 0; i < n; i++) {
        g.add_edge(n, i, 0);
    }
    // Bellman-Ford
    vector<int> h, p;
    if (!bellman_ford(g, n, h, p))
        throw std::invalid_argument("Negative cycle in graph");
    // Remove the virtual vertex
    g.vertices.pop_back();
    // Dijkstra
    for (int i = 0; i < n; i++) {
        Edge *e = g.vertices[i].head;
        while (e != nullptr) {
            e->weight += h[i] - h[e->dst];
            e         = e->next;
        }
    }
    dist.resize(n, vector<int>(n));
    prev.resize(n, vector<int>(n));
    for (int i = 0; i < n; i++) {
        dijkstra(g, i, dist[i], prev[i]);
        for (int j = 0; j < n; j++) {
            dist[i][j] += h[j] - h[i];
        }
    }
}

void print_path(vector<vector<int>> &prev, int src, int dst, ofstream &fout)
{
    if (src == dst) {
        fout << src;
    } else {
        print_path(prev, src, prev[src][dst], fout);
        fout << "," << dst;
    }
}

int main()
{
    ofstream time(time_file);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 2; j++) {
            ifstream fin(input_files[i][j]);
            Graph    g(num_vertices[i]);
            // Read the graph
            int src, dst, weight;
            while (!fin.eof()) {
                fin >> src >> dst >> weight;
                g.add_edge(src, dst, weight);
            }
            fin.close();

            // Run Johnson algorithm and time it
            auto                start = now();
            vector<vector<int>> dist, prev;
            johnson(g, dist, prev);
            auto end      = now();
            ns   duration = end - start;
            time << "(" << num_vertices[i] << ", " << num_edges[i][j]
                 << "): " << duration.count() << "ns" << endl;

            // Output the result
            ofstream fout(result_files[i][j]);
            for (int u = 0; u < num_vertices[i]; u++) {
                for (int v = 0; v < num_vertices[i]; v++) {
                    if (u == v)
                        continue;
                    fout << "(";
                    if (prev[u][v] != -1) {
                        print_path(prev, u, v, fout);
                        fout << " " << dist[u][v] << ")" << endl;
                    } else {
                        fout << "no path from " << u << " to " << v << ")"
                             << endl;
                    }
                }
            }
            fout.close();
        }
    }
}
