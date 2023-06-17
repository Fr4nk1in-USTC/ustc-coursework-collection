#!/usr/bin/env python


def selection_sort(A: list):
    for j in range(len(A) - 1):
        min = j
        for i in range(j + 1, len(A)):
            if A[i] < A[min]:
                min = i
        A[j], A[min] = A[min], A[j]


def test(A: list):
    selection_sort(A)
    assert sorted(A)


if __name__ == "__main__":
    test([2, 4, 1, 3, 5, 8, 6, 7])
