#include <stdlib.h>
#include <check.h>
#include "hashtable.h"


hashtable *ht;

void setup(void)
{
    ht_init(&ht, NULL, NULL);
}

void teardown(void)
{
    ht_free(ht);
}

START_TEST (test_ht_insert)
{
    int res, data;
    int *data_retrieved;

    res = ht_insert(ht, "test", &data);
    fail_unless(res == HT_OK);

    data_retrieved = ht_get(ht, "test");
    fail_unless(data_retrieved == &data);
}
END_TEST

START_TEST (test_ht_set)
{
    int res, data;
    int *data_retrieved;

    res = ht_set(ht, "test", &data);
    fail_unless(res == HT_OK);

    data_retrieved = ht_get(ht, "test");
    fail_unless(data_retrieved == &data);
}
END_TEST

START_TEST (test_ht_set_twice)
{
    int res, data1, data2;
    int *data_retrieved;

    res = ht_set(ht, "test", &data1);
    fail_unless(res == HT_OK);

    res = ht_set(ht, "test", &data2);
    fail_unless(res == HT_OK);

    data_retrieved = ht_get(ht, "test");
    fail_unless(data_retrieved == &data2);
}
END_TEST

Suite *ht_simple_suite(void)
{
    Suite *s = suite_create("hashtable operations with small amounts of (static) data");

    TCase *tc_simple = tcase_create("simple");

    tcase_add_checked_fixture (tc_simple, setup, teardown);

    tcase_add_test(tc_simple, test_ht_insert);
    tcase_add_test(tc_simple, test_ht_set);
    tcase_add_test(tc_simple, test_ht_set_twice);

    suite_add_tcase(s, tc_simple);

    return s;
}

/*
int main(void)
{
    int number_failed;
    Suite *s = ht_simple_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);

    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
*/
