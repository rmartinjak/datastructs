#include <stdlib.h>
#include <check.h>
#include "hashtable.h"


hashtable *ht;

static void setup(void)
{
    ht_init(&ht, NULL, NULL);
}

static void teardown(void)
{
    ht_free(ht);
}

START_TEST (test_ht_args_insert_get)
{
    int res, data;
    int *p;
    size_t sz = 3;

    res = ht_insert_a(ht, "test", &data, &sz, &sz);
    fail_unless(res == HT_OK);

    p = ht_get_a(ht, "tes", &sz, &sz);
    fail_unless(p == &data);

    p = ht_get_a(ht, "test", &sz, &sz);
    fail_unless(p == &data);

    p = ht_get(ht, "test");
    fail_unless(p == NULL);
}
END_TEST

START_TEST (test_ht_args_set_get)
{
    int res, data;
    int *p;
    size_t sz = 3;

    res = ht_set_a(ht, "test", &data, &sz, &sz);
    fail_unless(res == HT_OK);

    /* p != &data here because the key is NOT a
       "copy of the string" but a generic(!) pointer */
    p = ht_get(ht, "tes");
    fail_unless(p != &data);

    p = ht_get_a(ht, "tes", &sz, &sz);
    fail_unless(p == &data);

    p = ht_get_a(ht, "test", &sz, &sz);
    fail_unless(p == &data);

    p = ht_get(ht, "test");
    fail_unless(p == NULL);
}
END_TEST

Suite *ht_args_suite(void)
{
    Suite *s = suite_create("passing additional argument to hash and cmp functions");

    TCase *tc_simple = tcase_create("args");

    tcase_add_checked_fixture (tc_simple, setup, teardown);

    tcase_add_test(tc_simple, test_ht_args_insert_get);
    tcase_add_test(tc_simple, test_ht_args_set_get);

    suite_add_tcase(s, tc_simple);

    return s;
}
