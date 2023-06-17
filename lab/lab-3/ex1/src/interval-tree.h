#ifndef INT_TREE_H
#define INT_TREE_H

#include <ostream>

class IntervalNode
{
  public:
    IntervalNode *left;
    IntervalNode *right;
    IntervalNode *parent;

    bool red;

    int low;
    int high;
    int max;

    IntervalNode(int, int, IntervalNode *, IntervalNode *);
    ~IntervalNode();

    bool overlap(const IntervalNode &other) const;
    bool overlap(int, int) const;

    bool operator<(const IntervalNode &other) const;
    bool operator>(const IntervalNode &other) const;
    bool operator==(const IntervalNode &other) const;
};

class IntervalTree
{
  public:
    IntervalNode *root;
    IntervalNode *nil;

    IntervalTree();
    ~IntervalTree();

    IntervalNode *insert(IntervalNode *);
    IntervalNode *insert(int, int);

    IntervalNode *remove(int, int);
    IntervalNode *remove(IntervalNode *);

    IntervalNode *search(int, int);

    IntervalNode *exact_search(int, int);

    IntervalNode *minimum(IntervalNode *);
    IntervalNode *maximum(IntervalNode *);

    void inorder_output(std::ostream &) const;
    void to_tikz(std::ostream &) const;

  private:
    void left_rotate(IntervalNode *);
    void right_rotate(IntervalNode *);
    bool update_max(IntervalNode *);
    void insert_fixup(IntervalNode *);
    void remove_fixup(IntervalNode *);
    void transplant(IntervalNode *, IntervalNode *);
};

#endif
