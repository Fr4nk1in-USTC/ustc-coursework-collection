#include "interval-tree.h"

#include <functional>
#include <iostream>
#include <ostream>
#include <string>

using std::endl;

/********************* Interval Tree Node Implementation *********************/
IntervalNode::IntervalNode(int low, int high, IntervalNode *nil = nullptr,
                           IntervalNode *parent = nullptr):
    left(nil),
    right(nil),
    parent(parent),
    red(true),
    low(low),
    high(high),
    max(high)
{}

IntervalNode::~IntervalNode() {}

bool IntervalNode::operator<(const IntervalNode &other) const
{
    return low < other.low || (low == other.low && high < other.high);
}

bool IntervalNode::operator>(const IntervalNode &other) const
{
    return low > other.low || (low == other.low && high > other.high);
}

bool IntervalNode::operator==(const IntervalNode &other) const
{
    return low == other.low && high == other.high;
}

bool IntervalNode::overlap(const IntervalNode &other) const
{
    return overlap(other.low, other.high);
}

bool IntervalNode::overlap(int low, int high) const
{
    return low <= this->high && high >= this->low;
}

/************************ Interval Tree Implementation ***********************/
IntervalTree::IntervalTree()
{
    nil         = new IntervalNode(0, 0);
    nil->left   = nil;
    nil->right  = nil;
    nil->parent = nil;
    nil->red    = false;
    root        = nil;
}

IntervalTree::~IntervalTree()
{
    std::function<void(IntervalNode *)> traverse;
    traverse = [&](IntervalNode *node) {
        if (node == nil) {
            return;
        }
        traverse(node->left);
        traverse(node->right);
        delete node;
    };
    traverse(root);
    delete nil;
}

/**
 * Update the max value of the node and return true if the max value updated.
 */
bool IntervalTree::update_max(IntervalNode *node)
{
    int max = node->high;
    if (node->left != nil && node->left->max > max) {
        max = node->left->max;
    }
    if (node->right != nil && node->right->max > max) {
        max = node->right->max;
    }
    if (max != node->max) {
        node->max = max;
        return true;
    }
    return false;
}

IntervalNode *IntervalTree::minimum(IntervalNode *node)
{
    while (node->left != nil)
        node = node->left;
    return node;
}

IntervalNode *IntervalTree::maximum(IntervalNode *node)
{
    while (node->right != nil)
        node = node->right;
    return node;
}

/**
 * Search a node that overlaps with the given interval. Return `nullptr` if no
 * such node found.
 */
IntervalNode *IntervalTree::search(int low, int high)
{
    IntervalNode *node = root;
    while (node != nil) {
        if (node->overlap(low, high)) {
            return node;
        }
        if (node->left != nil && node->left->max >= low) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return nullptr;
}

/**
 * Search a node that exactly matches the given interval. Return `nullptr` if no
 * such node found.
 */
IntervalNode *IntervalTree::exact_search(int low, int high)
{
    IntervalNode *node = root;
    while (node != nil) {
        if (node->low == low && node->high == high)
            return node;
        if (low > node->low || (low == node->low && high > node->high)) {
            node = node->right;
        } else {
            node = node->left;
        }
    }
    return nullptr;
}

/**
 * Insert a new interval into the tree. Return the inserted node.
 */
IntervalNode *IntervalTree::insert(int low, int high)
{
    IntervalNode *parent = nil;
    for (IntervalNode *node = root; node != nil;) {
        parent = node;
        if (node->low > low || (node->low == low && node->high > high))
            node = node->left;
        else
            node = node->right;
    }
    IntervalNode *node = new IntervalNode(low, high, nil, parent);
    if (parent == nil)
        root = node;
    else if (*node < *parent)
        parent->left = node;
    else
        parent->right = node;

    while (parent != nil) {
        if (!update_max(parent))
            break;
        parent = parent->parent;
    }
    insert_fixup(node);
    return node;
}

/**
 * Insert a node into the tree.
 */
IntervalNode *IntervalTree::insert(IntervalNode *node)
{
    IntervalNode *parent = nil;
    for (IntervalNode *n = root; n != nil;) {
        parent = n;
        if (node < n)
            n = n->left;
        else
            n = n->right;
    }
    node->parent = parent;
    if (parent == nil)
        root = node;
    else if (*node < *parent)
        parent->left = node;
    else
        parent->right = node;
    node->left  = nil;
    node->right = nil;
    node->red   = true;

    while (parent != nil) {
        if (!update_max(parent))
            break;
        parent = parent->parent;
    }

    insert_fixup(node);
    return node;
}

/**
 * Remove a node from the tree. Behavior is undefined if the node is not in the
 * tree.
 */
IntervalNode *IntervalTree::remove(IntervalNode *node)
{
    IntervalNode *replacement;
    bool          old_red = node->red;
    if (node->left == nil) {
        replacement = node->right;
        transplant(node, node->right);
    } else if (node->right == nil) {
        replacement = node->left;
        transplant(node, node->left);
    } else {
        IntervalNode *successor = minimum(node->right);
        old_red                 = successor->red;
        replacement             = successor->right;
        if (successor->parent == node) {
            // Critical if `replacement` is `nil`.
            replacement->parent = successor;
        } else {
            transplant(successor, successor->right);
            successor->right         = node->right;
            successor->right->parent = successor;
        }
        successor->left         = node->left;
        successor->left->parent = successor;
        successor->red          = node->red;
        transplant(node, successor);
    }
    if (!old_red)
        remove_fixup(replacement);
    return node;
}

/**
 * Remove the node with given interval. No action is taken if the interval is
 * not in the tree and `nullptr` is returned.
 */
IntervalNode *IntervalTree::remove(int low, int high)
{
    IntervalNode *node = exact_search(low, high);
    if (node != nullptr)
        return remove(node);
    return nullptr;
}

/**
 * Output the tree to stream by inoder traversal.
 */
void IntervalTree::inorder_output(std::ostream &os) const
{
    std::function<void(IntervalNode *, std::ostream &)> inorder_recursive =
        [&](IntervalNode *node, std::ostream &os) {
        if (node == nil)
            return;
        inorder_recursive(node->left, os);
        os << node->low << " " << node->high << " " << node->max << " "
           << std::endl;
        inorder_recursive(node->right, os);
    };
    inorder_recursive(root, os);
}

/**
 * Output the tree to LaTeX TikZ code.
 */
void IntervalTree::to_tikz(std::ostream &os) const
{
    // LaTeX header.
    os << "\\documentclass[varwidth=\\maxdimen]{standalone}" << endl
       << "\\usepackage{tikz}" << endl
       << "\\usetikzlibrary{shapes}" << endl
       << "\\tikzset{" << endl
       << "    every node/.style={" << endl
       << "            rectangle split," << endl
       << "            rectangle split parts=2," << endl
       << "            draw," << endl
       << "            inner sep=.1em," << endl
       << "            align=center" << endl
       << "        }," << endl
       << "    red/.style={fill=red!20}," << endl
       << "    black/.style={fill=black!20}," << endl
       << "    level/.style={level distance=3em}," << endl
       << "    level 1/.style={sibling distance=64em}," << endl
       << "    level 2/.style={sibling distance=32em}," << endl
       << "    level 3/.style={sibling distance=16em}," << endl
       << "    level 4/.style={sibling distance=8em}," << endl
       << "    level 5/.style={sibling distance=4em}," << endl
       << "}" << endl
       << "\\begin{document}" << endl
       << "\\begin{tikzpicture}" << endl;
    // TikZ code
    std::function<void(IntervalNode *, std::ostream &)> to_tikz_recursive =
        [&](IntervalNode *node, std::ostream &os) {
        if (node == nil) {
            os << "node[draw=none] {} edge from parent[draw=none]" << endl;
            return;
        }
        if (node->red) {
            os << "node[red] {$[" << node->low << ", " << node->high
               << "]$\\nodepart{second}" << node->max << "}" << endl;
        } else {
            os << "node[black] {$[" << node->low << ", " << node->high
               << "]$\\nodepart{second}" << node->max << "}" << endl;
        }
        os << "child {" << endl;
        to_tikz_recursive(node->left, os);
        os << "}" << endl;
        os << "child {" << endl;
        to_tikz_recursive(node->right, os);
        os << "}" << endl;
    };
    os << "\\";
    to_tikz_recursive(root, os);
    os << ";" << endl
       << "\\end{tikzpicture}" << endl
       << "\\end{document}" << endl;
}

void IntervalTree::left_rotate(IntervalNode *node)
{
    IntervalNode *right = node->right;
    node->right         = right->left;
    if (right->left != nil)
        right->left->parent = node;
    right->parent = node->parent;
    if (node->parent == nil)
        root = right;
    else if (node == node->parent->left)
        node->parent->left = right;
    else
        node->parent->right = right;
    right->left  = node;
    node->parent = right;
    update_max(node);
    update_max(right);
}

void IntervalTree::right_rotate(IntervalNode *node)
{
    IntervalNode *left = node->left;
    node->left         = left->right;
    if (left->right != nil)
        left->right->parent = node;
    left->parent = node->parent;
    if (node->parent == nil)
        root = left;
    else if (node == node->parent->right)
        node->parent->right = left;
    else
        node->parent->left = left;
    left->right  = node;
    node->parent = left;
    update_max(node);
    update_max(left);
}

void IntervalTree::insert_fixup(IntervalNode *node)
{
    IntervalNode *parent = node->parent;
    IntervalNode *grandparent;
    IntervalNode *uncle;
    while (parent->red) {
        grandparent = parent->parent;
        if (parent == grandparent->left) {
            uncle = grandparent->right;
            if (uncle->red) {
                parent->red      = false;
                uncle->red       = false;
                grandparent->red = true;
                node             = grandparent;
                parent           = node->parent;
            } else if (node == parent->right) {
                node = parent;
                left_rotate(node);
                parent = node->parent;
            } else {
                parent->red      = false;
                grandparent->red = true;
                right_rotate(grandparent);
            }
        } else {
            uncle = grandparent->left;
            if (uncle->red) {
                parent->red      = false;
                uncle->red       = false;
                grandparent->red = true;
                node             = grandparent;
                parent           = node->parent;
            } else if (node == parent->left) {
                node = parent;
                right_rotate(node);
                parent = node->parent;
            } else {
                parent->red      = false;
                grandparent->red = true;
                left_rotate(grandparent);
            }
        }
    }
    root->red = false;
}

void IntervalTree::transplant(IntervalNode *old, IntervalNode *replacement)
{
    if (old->parent == nil)
        root = replacement;
    else if (old == old->parent->left)
        old->parent->left = replacement;
    else
        old->parent->right = replacement;
    replacement->parent = old->parent;
    // Update max from parent to root.
    IntervalNode *node = old->parent;
    while (node != nil && update_max(node))
        node = node->parent;
}

void IntervalTree::remove_fixup(IntervalNode *node)
{
    IntervalNode *sibling;
    while (node != root && !node->red) {
        if (node == node->parent->left) {
            sibling = node->parent->right;
            if (sibling->red) {
                sibling->red      = false;
                node->parent->red = true;
                left_rotate(node->parent);
                sibling = node->parent->right;
            }
            if (!sibling->left->red && !sibling->right->red) {
                sibling->red = true;
                node         = node->parent;
            } else {
                if (!sibling->right->red) {
                    sibling->left->red = false;
                    sibling->red       = true;
                    right_rotate(sibling);
                    sibling = node->parent->right;
                }
                sibling->red        = node->parent->red;
                node->parent->red   = false;
                sibling->right->red = false;
                left_rotate(node->parent);
                node = root;
            }
        } else {
            sibling = node->parent->left;
            if (sibling->red) {
                sibling->red      = false;
                node->parent->red = true;
                right_rotate(node->parent);
                sibling = node->parent->left;
            }
            if (!sibling->left->red && !sibling->right->red) {
                sibling->red = true;
                node         = node->parent;
            } else {
                if (!sibling->left->red) {
                    sibling->right->red = false;
                    sibling->red        = true;
                    left_rotate(sibling);
                    sibling = node->parent->left;
                }
                sibling->red       = node->parent->red;
                node->parent->red  = false;
                sibling->left->red = false;
                right_rotate(node->parent);
                node = root;
            }
        }
    }
    node->red = false;
}
