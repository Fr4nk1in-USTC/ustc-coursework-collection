#include <mpi.h>
#include <mpi_proto.h>
#include <stdio.h>

int nums[] = {1, 2, 3, 4, 5, 6, 7, 8};

int tree(int number)
{
    int id, num_procs;

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int        recv;
    MPI_Status status;

    /* Assuming smaller proc be the parent of larger proc */
    /* Bottom-up */
    for (int i = 1; i < num_procs; i <<= 1) {
        int tag     = i;
        int sibling = id ^ tag;
        if (id & tag) {
            MPI_Send(&number, 1, MPI_INT, sibling, tag, MPI_COMM_WORLD);
        } else {
            MPI_Recv(&recv, 1, MPI_INT, sibling, tag, MPI_COMM_WORLD, &status);
            number += recv;
        }
    }

    /* Top-down */
    for (int i = num_procs; i >= 2; i >>= 1) {
        if ((id & (i - 1)) == 0) {  // id % i == 0
            MPI_Send(&number, 1, MPI_INT, id + i / 2, i + num_procs,
                     MPI_COMM_WORLD);
        } else if ((id & (i / 2 - 1)) == 0) {  // id % (i / 2) == 0
            MPI_Recv(&number, 1, MPI_INT, id - i / 2, i + num_procs,
                     MPI_COMM_WORLD, &status);
        }
    }

    return number;
}

int main(int argc, char *argv[])
{
    int id, num_procs;
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int number = nums[id];

    printf("[Info] Proc #%d's origin number is %d\n", id, number);

    MPI_Barrier(MPI_COMM_WORLD);

    int tree_sum = tree(number);

    printf("[Info] Proc #%d's tree sum is %d.\n", id, tree_sum);

    MPI_Finalize();
}
