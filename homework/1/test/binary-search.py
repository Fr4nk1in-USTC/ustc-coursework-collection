#!/usr/bin/env python

def binary_search(A: list, v, p: int, r: int):
    if p > r:
        return None
    q = (p + r) // 2
    if v == A[q]:
        return q
    elif v > A[q]:
        return binary_search(A, v, q + 1, r)
    else:
        return binary_search(A, v, p, q - 1)


def test(A: list, v):
    i = binary_search(A, v, 0, len(A) - 1)
    if i == None:
        assert v not in A
    else:
        assert A[i] == v


if __name__ == "__main__":
    test([1, 2, 3, 4, 5, 6, 7, 8], 9)
    test([1, 2, 3, 4, 5, 6, 7, 8], 4)
