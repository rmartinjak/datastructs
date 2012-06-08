#include <limits.h>

#if __STDC_VERSION__ >= 199901L
#include <stdint.h>
#include <inttypes.h>
#else
typedef unsigned long uintmax_t;
#define UINTMAX_MAX ULONG_MAX
#define PRIuMAX "lu"
#endif

uintmax_t shift_mod(uintmax_t a, unsigned int sh, uintmax_t mod);
uintmax_t mult_mod(uintmax_t a, uintmax_t b, uintmax_t mod);
uintmax_t pow_mod(uintmax_t base, uintmax_t exp, uintmax_t mod);
uintmax_t randumax(uintmax_t min, uintmax_t max);

int isprime_mr(uintmax_t n);
