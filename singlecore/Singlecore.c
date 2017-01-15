#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

clock_t t1, t2;
double primesqrt;
float secs, diff;
int stepquota, quota, count = 1, intervals;

double is_prime(double prime) {
    if (prime == 1) return 0;

    primesqrt = sqrt(prime);

    if (primesqrt==floor(primesqrt)) return 0;

    for (double i = 3; i < primesqrt; i+=2) if ((floor(prime/i)*i) == prime) return 0;

    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <compuation quota> <file to write to>\n", argv[0]);
        return 0;
    }

    FILE *file;
    file = fopen(argv[2], "a");

    t1 = clock();

    quota = atoi(argv[1]);

    for (int prime = 1; count <= quota; prime+=2) { 
        if(is_prime(prime)) count++;
    }

    t2=clock();
    diff = ((float)t2-(float)t1);
    secs = diff / CLOCKS_PER_SEC;
    fprintf(file, "%f ", secs);
}
