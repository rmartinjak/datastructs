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

START_TEST (test_ht_insert)
{
    int res, data;
    int *p;

    res = ht_insert(ht, "test", &data);
    fail_unless(res == HT_OK,
        "inserting a new item should return HT_OK");

    p = ht_get(ht, "test");
    fail_unless(p == &data,
        "ht_get() should return the data inserted by ht_insert()");
}
END_TEST

START_TEST (test_ht_insert_null)
{
    int res;
    void *p;

    res = ht_insert(ht, "test", NULL);
    fail_unless(res == HT_OK,
        "inserting a new item with NULL as data should return HT_OK");

    p = ht_get(ht, "test");
    fail_unless(p == NULL,
        "ht_get() should return the NULL inserted by ht_insert()");
}
END_TEST

START_TEST (test_ht_insert_twice)
{
    int res, data1, data2;
    int *p;

    res = ht_insert(ht, "test", &data1);
    fail_unless(res == HT_OK,
        "inserting a new item should return HT_OK");

    res = ht_insert(ht, "test", &data2);
    fail_unless(res == HT_EXIST,
        "re-inserting an item should return HT_EXIST");

    p = ht_get(ht, "test");
    fail_unless(p == &data1,
        "ht_get() should return the data set by the first ht_insert() call");
}
END_TEST

START_TEST (test_ht_insert_remove)
{
    int data, *p;

    ht_insert(ht, "test", NULL);
    ht_remove(ht, "test");
    fail_unless(ht_empty(ht),
        "after inserting and removing an item with the same key to an empty"
        "hashtable, it should be empty again");


    ht_insert(ht, "foo", &data);
    ht_insert(ht, "bar", &data);
    p = ht_remove(ht, "foo");
    fail_unless(p == &data,
        "ht_remove() should return the data associated with the given key");

    p = ht_get(ht, "foo");
    fail_unless(p != &data,
        "removed item should no longer be returned by ht_get()");

    p = ht_get(ht, "bar");
    fail_unless(p == &data,
        "a not removed item should still be in the hashtable if another"
        "item was removed");
}
END_TEST

START_TEST (test_ht_set)
{
    int res, data;
    int *p;

    res = ht_set(ht, "test", &data);
    fail_unless(res == HT_OK,
        "setting a new item should return HT_OK");

    p = ht_get(ht, "test");
    fail_unless(p == &data,
        "ht_get() should return the data set by ht_set()");
}
END_TEST

START_TEST (test_ht_set_twice)
{
    int res, data1, data2;
    int *p;

    res = ht_set(ht, "test", &data1);
    fail_unless(res == HT_OK,
        "setting a new item should return HT_OK");

    res = ht_set(ht, "test", &data2);
    fail_unless(res == HT_OK,
        "re-setting an item should return HT_OK");

    p = ht_get(ht, "test");
    fail_unless(p == &data2,
        "ht_get() should return the data set by the last ht_set() call");
}
END_TEST

START_TEST (test_ht_get_nonexistent)
{
    void *p;

    p = ht_get(ht, "test");
    fail_unless(p == NULL,
        "ht_get() should return NULL if non-existent key passed");
}
END_TEST

START_TEST (test_ht_remove_nonexistent)
{
    void *p;

    p = ht_remove(ht, "test");
    fail_unless(p == NULL,
        "ht_remove() should return NULL if non-existent key passed");
}
END_TEST

Suite *ht_simple_suite(void)
{
    Suite *s = suite_create("hashtable operations with small amounts of (static) data");

    TCase *tc_simple = tcase_create("simple");

    tcase_add_checked_fixture (tc_simple, setup, teardown);

    tcase_add_test(tc_simple, test_ht_insert);
    tcase_add_test(tc_simple, test_ht_insert_null);
    tcase_add_test(tc_simple, test_ht_insert_twice);
    tcase_add_test(tc_simple, test_ht_insert_remove);
    tcase_add_test(tc_simple, test_ht_set);
    tcase_add_test(tc_simple, test_ht_set_twice);
    tcase_add_test(tc_simple, test_ht_get_nonexistent);
    tcase_add_test(tc_simple, test_ht_remove_nonexistent);

    suite_add_tcase(s, tc_simple);

    return s;
}
