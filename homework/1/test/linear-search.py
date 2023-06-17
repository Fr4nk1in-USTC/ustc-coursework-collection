#!/usr/bin/env python

def linear_search(A: list, v):
    i = None
    for j in range(len(A)):
        if A[j] == v:
            i = j
            return i
    return i


def test(A: list, v):
    i = linear_search(A, v)
    if i == None:
        assert v not in A
    else:
        assert A[i] == v


if __name__ == "__main__":
    test([2, 4, 2, 1, 5, 7, 6], 9)
    test([3, 5, 7, 9, 2, 4, 1, 4], 4)
