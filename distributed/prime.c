#include <stdio.h>
#include </usr/include/mpich/mpi.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


double is_prime(double prime) {
    //printf("testing %f\n", prime);
    if (prime == 1) return 0;
    if (prime == 3 || prime == 5) return 1;

    int primesqrt = sqrt(prime);
    /*
       if (primesqrt==floor(primesqrt)) {
       printf("sqrt\n");
       return 0;
       }
       */
    for (double i = 3; i < primesqrt; i+=2) if ((floor(prime/i)*i) == prime) return 0;

    return 1;
}

int main(int argc, char** argv) {
    int rank, total, count=0;

    FILE *file;

    char filename[1024];

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &total);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    //MPI_Comm slaves;
    //MPI_Comm_split(MPI_COMM_WORLD, (rank == 0), rank, &slaves); 

    if (argc != 2) {
        if (rank == 0) printf("Usage: %s <computation quota>\n", argv[0]);
        MPI_Finalize();
        return 0;
    }

    rank++;

    sprintf(filename, "./numberlists/numberlist%d.txt", rank);
    file = fopen(filename, "w");

    double quota = atof(argv[1]);

    if (fmod(quota, total) != 0) {
        if (rank == 1) {
            quota = floor(quota/total)+fmod(quota, total);   
        }
        else {
            quota = floor(quota/total);
        }
    }
    else {
        quota = quota/total;
    }

    printf("P: %d of %d checking in, quota: %f\n", rank, total, quota);

    //printf("%f\n", quota);

    double start = ((rank*2)+1);
    for (double prime = start; count < quota; prime+=total) {
        if (is_prime(prime)) {
            //printf("%d, gucc'd\n", rank);
            count++;
            fprintf(file, "%.0f\n", prime);
        }
    }

    printf("node %d done...\n", rank);

    MPI_Finalize();

    //MPI_Barrier(slaves);
    //if (rank==1) printf("done\n");
    return 0;
}
