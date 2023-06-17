#!/usr/bin/env python

def max_subarray(A: list[int]) -> tuple[int, int, int]:
    if A == []:
        return 0, 0, 0
    max_sum = -int(1e9)
    max_left = -1
    max_right = -1
    max_tail_sum = 0
    tail_left = 0

    for i, a in enumerate(A):
        max_tail_sum += a
        if max_tail_sum > max_sum:
            max_sum = max_tail_sum
            max_left = tail_left
            max_right = i
        if max_tail_sum < 0:
            max_tail_sum = 0
            tail_left = i + 1

    return max_left, max_right, max_sum


if __name__ == "__main__":
    print(max_subarray([13, -3, -25, 20, -3, -16, -
          23, 18, 20, -7, 12, -5, -22, 15, -4, 7]))
