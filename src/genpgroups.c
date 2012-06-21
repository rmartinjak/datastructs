#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "mr.h"

int main(int argc, char **argv)
{
    size_t c;
    int i;
    int count;

    srand(time(NULL));

    for (i = 0; i < PGROUP_COUNT; i++) {
        printf("unsigned long primes%u[] = { ", i);
        count = PGROUP_ELEMENTS;
        while (count--)
        {
            do c = randumax(1 << (i+10), 1 << (i+11));
            while (!isprime_mr(c));

            printf(count ? "%u, " : "%u ", c);
        }
        printf("};\n");
    }

    printf("unsigned long *primes[PGROUP_COUNT] = { ");
    for (i = 0; i < PGROUP_COUNT-1; i++)
        printf("primes%u, ", i);

    printf("primes%u };\n", PGROUP_COUNT-1);

    return 0;
}
