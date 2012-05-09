#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <limits.h>
#include <time.h>

#if _ISOC99_SOURCE
#define longint long long
#define ULONGINT_MAX ULLONG_MAX
#else
/* typedef long int longint; */
#define longint long
#define ULONGINT_MAX ULONG_MAX
#endif

#define LONGINT_BIT (sizeof(longint) * CHAR_BIT)

#define EVEN(n) (!(n % 2))
#define WITNESSES 100

unsigned longint shift_mod(unsigned longint a, unsigned int sh, unsigned longint mod);
unsigned longint mult_mod(unsigned longint a, unsigned longint b, unsigned longint mod);
unsigned longint pow_mod(unsigned longint base, unsigned longint exp, unsigned longint mod);

unsigned longint randulongint(unsigned longint min, unsigned longint max);

int millerrabin(unsigned longint n);


unsigned longint shift_mod(unsigned longint a, unsigned int sh, unsigned longint mod)
{
    while (sh--) {
        a <<= 1;
        a %= mod;
    }
    return a;
}

unsigned longint mult_mod(unsigned longint a, unsigned longint b, unsigned longint mod)
{
    unsigned longint ret = 0;
    unsigned int sh = 0;

    while (b) {
        if (b & 1) {
            ret += shift_mod(a, sh, mod);
            ret %= mod;
        }
        b >>= 1;
        sh++;
    }
    return ret;
}

unsigned longint pow_mod(unsigned longint base, unsigned longint exp, unsigned longint mod)
{
    unsigned longint ret = 1;
    unsigned longint b;

    b = base;

    while (exp)
    {
        if (exp & 1) {
            ret = mult_mod(ret, b, mod);
        }
        exp >>= 1;
        b = mult_mod(b, b, mod);
    }
    return (unsigned longint)ret;
}

unsigned longint randulongint(unsigned longint min, unsigned longint max)
{
    static size_t rand_bits = 0;
    size_t bits;
    unsigned longint rnd = 0;

    if (!rand_bits)
    {
        int r;
        for (r = RAND_MAX; r > 0; r >>= 1)
            rand_bits++;
    }

    for (bits = 0; bits < LONGINT_BIT; bits += rand_bits)
    {
        rnd <<= rand_bits;
        rnd |= rand();
    }

    rnd = min + max * ((double)rnd / ULONGINT_MAX);
    return rnd;
}

int millerrabin(unsigned longint n)
{
    unsigned longint d, s, i, a;
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
        a = randulongint(2, n-2);
        r1 = 0;
        r2 = 0;

        /* first test */
        if (pow_mod(a, d, n) == 1) {
            r1 = 1;
        }

        /* second test */
        for (i = 0; i < s; i++) {
            if (pow_mod(a, (1 << i) * d, n) == n-1) {
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

int main(int argc, char **argv)
{

    size_t c;
    int i;
    int count;

    srand(time(NULL));

    for (i = 0; i < PGROUP_COUNT; i++) {
        printf("unsigned long primes%u[] = { ", i);
        count = PGROUP_ELEMENTS;
        while (count--) {
            do {
                c = randulongint(1 << (i+10), 1 << (i+11));
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
