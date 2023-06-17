#!/usr/bin/env python

def binary_add(A: list[int], B: list[int]) -> list[int]:
    carry = 0
    n = len(A)
    C = [0] * (n + 1)
    for i in range(n - 1, -1, -1):
        C[i + 1] = A[i] + B[i] + carry
        if C[i + 1] >= 2:
            C[i + 1] = C[i + 1] - 2
            carry = 1
        else:
            carry = 0
    C[0] = carry
    return C


def bin_list_to_int(A: list[int]) -> int:
    result = 0
    for i in A:
        result += i
        result <<= 1
    return result


def test(A: list[int], B: list[int]):
    if all(0 <= i < 2 for i in A) and all(0 <= i < 2 for i in B):
        assert bin_list_to_int(A) + bin_list_to_int(B) \
            == bin_list_to_int(binary_add(A, B))
    else:
        assert False, "Input invalid!"


if __name__ == "__main__":
    test([1, 0, 1, 1], [1, 1, 0, 1])
