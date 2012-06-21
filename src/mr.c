/* miller-rabin prime number test */

#include "mr.h"

#include <stdlib.h>
#include <time.h>

#define EVEN(n) (!(n % 2))

#define WITNESSES 200

#define UINTMAX_BIT (sizeof(uintmax_t) * CHAR_BIT)


uintmax_t shift_mod(uintmax_t a, unsigned int sh, uintmax_t mod)
{
    while (sh--)
    {
        a <<= 1;
        a %= mod;
    }
    return a;
}

uintmax_t mult_mod(uintmax_t a, uintmax_t b, uintmax_t mod)
{
    uintmax_t ret = 0;
    unsigned int sh = 0;

    while (b)
    {
        if (b & 1)
        {
            ret += shift_mod(a, sh, mod);
            ret %= mod;
        }
        b >>= 1;
        sh++;
    }
    return ret;
}

uintmax_t pow_mod(uintmax_t base, uintmax_t exp, uintmax_t mod)
{
    uintmax_t ret = 1;
    uintmax_t b;

    b = base;

    while (exp)
    {
        if (exp & 1)
            ret = mult_mod(ret, b, mod);
        exp >>= 1;
        b = mult_mod(b, b, mod);
    }
    return (uintmax_t)ret;
}

uintmax_t randumax(uintmax_t min, uintmax_t max)
{
    static size_t rand_bits = 0;
    size_t bits;
    uintmax_t rnd = 0;

    if (!rand_bits)
    {
        int r;
        for (r = RAND_MAX; r > 0; r >>= 1)
            rand_bits++;
        srand(time(NULL));
    }

    for (bits = 0; bits < UINTMAX_BIT; bits += rand_bits)
    {
        rnd <<= rand_bits;
        rnd |= rand();
    }

    return min + (max - min + 1) * ((double)rnd / UINTMAX_MAX);
}

int isprime_mr(uintmax_t n)
{
    uintmax_t d, s, i, a;
    int k;
    int r1, r2;

    if (EVEN(n) || n <= 2)
        return 0;

    d = n-1;
    s = 0;
    while (EVEN(d))
    {
        s++;
        d >>= 1;
    }

    for (k = 0; k < WITNESSES; k++)
    {
        a = randumax(2, n-2);
        r1 = 0;
        r2 = 0;

        /* first test */
        if (pow_mod(a, d, n) == 1)
            r1 = 1;

        /* second test */
        for (i = 0; i < s; i++)
        {
            if (pow_mod(a, (1 << i) * d, n) == n-1)
            {
                r2 = 1;
                break;
            }
        }
        if (!r1 && !r2)
            return 0;
    }

    return 1;
}
