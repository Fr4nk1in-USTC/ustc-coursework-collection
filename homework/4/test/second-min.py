#!/usr/bin/env python

import math


def brother(k: int) -> int:
    return 4 * (k // 2) + 1 - k


def parent(k: int) -> int:
    return k // 2


def second_min(A: list[int]) -> int:
    # Note: the heap is 1-indexed
    n = len(A) - 1
    T = (2 * n + 2) * [0]
    S = (2 * n) * [0]
    T[2 * n] = T[2 * n + 1] = 1 << 31
    for i in range(1, n + 1):
        T[n + i - 1] = A[i]
        S[n + i - 1] = 2 * n

    for i in range(2 * n - 1, 1, -2):
        if T[i] < T[i - 1]:
            T[parent(i)] = T[i]
            S[parent(i)] = i
        else:
            T[parent(i)] = T[i - 1]
            S[parent(i)] = i - 1

    idx = S[1]
    sec_min = T[brother(idx)]
    for j in range(1, math.ceil(math.log2(n))):
        idx = S[idx]
        if T[brother(idx)] < sec_min:
            sec_min = T[brother(idx)]
    return sec_min


if __name__ == "__main__":
    print(second_min([0, 1, 7, 3, 4, 5, 6, 7, 8, 9, 10]))
