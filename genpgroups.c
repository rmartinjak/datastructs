#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>

#define WITNESSES 200
#define EVEN(n) (!(n & 1))

unsigned long ulpowmod(unsigned long base, unsigned long exp, unsigned long mod) {
    unsigned long long result = 1;
    unsigned long long b;

    b = base;

    while (exp)
    {
        if (exp & 1) {
            result = (result * b) % mod;
        }
        exp >>= 1;
        b = (b * b) % mod;
    }
    return (unsigned long)result;
}

unsigned long randlong(unsigned long min, unsigned long max) {
    unsigned long ret = 0;

    if (min > max) {
        printf("min: %lu, max: %lu\n", min, max);
        return 1452;
    }

    while (ret < min || ret > max) {
        ret = ((rand()*rand()) % (max-min)) + min;
    }

    return ret;
}


int millerrabin(unsigned long n) {
    unsigned long d, s, i, a;
    int k;
    int r1, r2;

    if (EVEN(n) || n <= 2)
        return 0;


    d = n-1;
    s = 0;
    while (EVEN(d)) {
        s++;
        d >>= 1;
    }

    for (k = 0; k < WITNESSES; k++) {
        a = randlong(2, n-2);
        r1 = 0;
        r2 = 0;

        /* first test */
        if (ulpowmod(a, d, n) == 1) {
            r1 = 1;
        }

        /* second test */
        for (i = 0; i < s; i++) {
            if (ulpowmod(a, (1 << i) * d, n) == n-1) {
                r2 = 1;
                break;
            }
        }
        if (!r1 && !r2) {
            return 0;
        }
    }

    return 1;
}


int main(int argc, char **argv) {

    size_t c;
    int i;
    int count;

    srand(time(NULL));

    for (i = 0; i < PGROUP_COUNT; i++) {
        printf("unsigned long primes%u[] = { ", i);
        count = PGROUP_ELEMENTS;
        while (count--) {
            do {
                c = randlong(1 << (i+10), 1 << (i+11)); 
            } while (!millerrabin(c));
            if (count)
                printf("%u, ", c);
            else
                printf("%u ", c);
        }
        printf("};\n");
    }

    printf("unsigned long *primes[PGROUP_COUNT] = { ");
    for (i = 0; i < PGROUP_COUNT-1; i++)
        printf("primes%u, ", i);

    printf("primes%u };\n", PGROUP_COUNT-1);

    return 0;
}
