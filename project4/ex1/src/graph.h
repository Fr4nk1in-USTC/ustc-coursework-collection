#ifndef GRAPH_H
#define GRAPH_H

#include <vector>

class Edge
{
  public:
    int dst;
    int weight;

    Edge *next;

    Edge(int dst, int weight): dst(dst), weight(weight), next(nullptr) {}

    ~Edge() {}
};

class Vertex
{
  public:
    Edge *head;

    Vertex(): head(nullptr) {}

    Vertex(const Vertex &v)
    {
        head    = nullptr;
        Edge *p = v.head;
        Edge *q = head;
        while (p != nullptr) {
            if (q == nullptr) {
                q    = new Edge(p->dst, p->weight);
                head = q;
            } else {
                q->next = new Edge(p->dst, p->weight);
                q       = q->next;
            }
            p = p->next;
        }
    }

    ~Vertex()
    {
        Edge *p = head;
        while (p != nullptr) {
            Edge *q = p;
            p       = p->next;
            delete q;
        }
    }

    void append(int dst, int weight)
    {
        if (head == nullptr) {
            head = new Edge(dst, weight);
        } else {
            Edge *p = new Edge(dst, weight);
            p->next = head;
            head    = p;
        }
    }

    void remove(int dst)
    {
        Edge *p = head;
        Edge *q = nullptr;
        while (p != nullptr) {
            if (p->dst == dst) {
                if (q == nullptr) {
                    head = p->next;
                } else {
                    q->next = p->next;
                }
                delete p;
                break;
            }
            q = p;
            p = p->next;
        }
    }
};

class Graph
{
  public:
    std::vector<Vertex> vertices;

    Graph(int num_vertices): vertices(num_vertices) {}

    Graph(const Graph &g) { vertices = g.vertices; }

    ~Graph() {}

    int size() const { return vertices.size(); }

    void add_edge(int src, int dst, int weight)
    {
        vertices[src].append(dst, weight);
    }

    void remove_edge(int src, int dst) { vertices[src].remove(dst); }
};

#endif
