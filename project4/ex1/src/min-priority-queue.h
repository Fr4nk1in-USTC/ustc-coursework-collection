#ifndef MIN_PRIORITY_QUEUE_H
#define MIN_PRIORITY_QUEUE_H

#include "config.h"

#include <stdexcept>
#include <vector>

inline int parent(int i)
{
    return (i - 1) / 2;
}

class MinPriorityQueue
{
  public:
    MinPriorityQueue(int size): size(size)
    {
        values.resize(size, inf);
        indices.resize(size);
        positions.resize(size);
        for (int i = 0; i < size; i++) {
            indices[i]   = i;
            positions[i] = i;
        }
    };

    ~MinPriorityQueue() {};

    bool empty() { return size == 0; }

    // Insert a new element to the queue, its index is `size`.
    void insert(int key)
    {
        if ((std::size_t)size == values.size()) {
            values.push_back(inf);
            indices.push_back(size);
            positions.push_back(size);
        } else {
            values[size]    = inf;
            indices[size]   = size;
            positions[size] = size;
        }
        decrease_key(size++, key);
    };

    // Return the value of the minimum element
    int minimum_value() const
    {
        if (size == 0)
            throw std::out_of_range("Empty queue, no minimum");
        return values[0];
    };

    // Return the index of the minimum element
    int minimum_index() const
    {
        if (size == 0)
            throw std::out_of_range("Empty queue, no minimum");
        return indices[0];
    };

    // Pop out the minimum element.
    // Return the corresponding index of the minimum element
    int extract_min()
    {
        if (size < 1)
            throw std::out_of_range("Empty queue, no minimum to extract");

        size--;
        int min_index = indices[0];

        positions[indices[0]]    = -1;
        positions[indices[size]] = 0;

        values[0]  = values[size];
        indices[0] = indices[size];
        heapify(0);
        return min_index;
    };

    // Decrease the value of the element of index `index` to `key`
    void decrease_key(int index, int key)
    {
        int i = positions[index];
        if (key > values[i])
            throw std::invalid_argument("New key is greater than current key");
        values[i] = key;
        while (i > 0 && values[parent(i)] > values[i]) {
            std::swap(positions[indices[i]], positions[indices[parent(i)]]);
            std::swap(values[i], values[parent(i)]);
            std::swap(indices[i], indices[parent(i)]);
            i = parent(i);
        }
    };

  private:
    int              size;
    std::vector<int> values;     // The values of the elements
    std::vector<int> indices;    // The indices of the elements
    std::vector<int> positions;  // The positions of the corresponding indices

    void heapify(int i)
    {
        int left     = 2 * i + 1;
        int right    = 2 * i + 2;
        int smallest = i;
        if (left < size && values[left] < values[smallest])
            smallest = left;
        if (right < size && values[right] < values[smallest])
            smallest = right;
        if (smallest != i) {
            std::swap(positions[indices[i]], positions[indices[smallest]]);
            std::swap(values[i], values[smallest]);
            std::swap(indices[i], indices[smallest]);
            heapify(smallest);
        }
    };
};

#endif
