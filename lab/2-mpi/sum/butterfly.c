#include <mpi.h>
#include <mpi_proto.h>
#include <stdio.h>

int nums[] = {1, 2, 3, 4, 5, 6, 7, 8};

int butterfly(int number)
{
    int id, num_procs;

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int        recv;
    MPI_Status status;
    for (int i = 1; i < num_procs; i <<= 1) {
        int tag  = i;
        int dest = id ^ tag;
        MPI_Send(&number, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
        MPI_Recv(&recv, 1, MPI_INT, dest, tag, MPI_COMM_WORLD, &status);
        number += recv;
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

    int butterfly_sum = butterfly(number);

    printf("[Info] Proc #%d's butterfly sum is %d.\n", id, butterfly_sum);

    MPI_Finalize();
}
