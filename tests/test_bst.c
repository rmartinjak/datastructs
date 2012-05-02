#include <stdlib.h>
#include <time.h>
#include <check.h>

#include "bst.h"

#define N 10000

bst *t;
long numbers[N];


static long random_number(void)
{
    size_t i;
    i = N * ((double)rand() / RAND_MAX);
    return numbers[i];
}

static void setup(void)
{
    long x;
    size_t i;

    t = bst_init();

    srand(time(NULL));

    for (i=0; i<N; ++i)
    {
        do {
            x = rand();
        } while (x == 1337);
        numbers[i] = x;
    }
}

static void teardown(void)
{
    bst_free(t, NULL);
}

START_TEST (test_bst_nodup)
{
    long x;
    size_t i, j;

    /* srand(time(NULL)); */

    for (i=0; i<N; ++i)
    {
        bst_insert(t, numbers[i], &numbers[i]);
        fail_unless(bst_contains(t, numbers[i]));

        if (i == N/2)
        {
            bst_insert(t, 1337, NULL);
            fail_unless(bst_contains(t, 1337));
        }
    }


    for (i=0; i<N; ++i)
    {
        for (j=0; j<20; ++j)
        {
            long *p;
            x = random_number();

            if (bst_contains(t, x))
            {
                p = bst_get(t, x);
                fail_unless(*p == x);
            }
        }

        fail_unless(bst_contains(t, numbers[i]));
        bst_remove(t, numbers[i], NULL);
        fail_unless(!bst_contains(t, numbers[i]));
    }

    fail_unless(bst_contains(t, 1337));
    bst_remove(t, 1337, NULL);
    fail_unless(!bst_contains(t, 1337));
}
END_TEST

Suite *bst_suite(void)
{
    Suite *s = suite_create("testing a bunch of numbers");

    TCase *tc_simple = tcase_create("simple");

    tcase_add_checked_fixture (tc_simple, setup, teardown);

    tcase_add_test(tc_simple, test_bst_nodup);

    suite_add_tcase(s, tc_simple);

    return s;
}

Suite *bst_suite(void);

int main(void)
{
    int number_failed;
    SRunner *sr = srunner_create(NULL);

    srunner_add_suite(sr, bst_suite());

    srunner_set_fork_status(sr, CK_NOFORK);

    srunner_run_all(sr, CK_NORMAL);

    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
