#include <stdlib.h>
#include <check.h>
#include "hashtable.h"

/*========================*/
/* test ht initialization */
/*========================*/

START_TEST (test_ht_init_no_hash_no_cmp)
{
    hashtable *ht;
    int res;
    
    res = ht_init(&ht, NULL, NULL);

    fail_unless(res == HT_OK && ht != NULL,
        "ht_init() should return HT_OK and put a hashtable pointer"
        "(not NULL) in the given **ht");

    ht_free(ht);
}
END_TEST

START_TEST (test_ht_init_no_hash)
{
    hashtable *ht;
    int res;

    res = ht_init(&ht, NULL, (ht_cmpfunc_t)42);

    fail_unless(res == HT_ERROR && ht == NULL,
        "ht_init() should return HT_ERROR and set ht to NULL"
        "if a cmp function is passed but no hash function");
}
END_TEST

START_TEST (test_ht_init_no_cmp)
{
    hashtable *ht;
    int res;

    res = ht_init(&ht, (ht_hashfunc_t)42, NULL);
    fail_unless(res == HT_ERROR && ht == NULL,
        "ht_init() should return HT_ERROR and set ht to NULL"
        "if a hash function is passed but no cmp function");
}
END_TEST

Suite *ht_init_suite(void)
{
    Suite *s = suite_create("hashtable initialization");
    TCase *tc_ht_init = tcase_create("ht_init");
    tcase_add_test(tc_ht_init, test_ht_init_no_hash_no_cmp);
    tcase_add_test(tc_ht_init, test_ht_init_no_hash);
    tcase_add_test(tc_ht_init, test_ht_init_no_cmp);
    suite_add_tcase(s, tc_ht_init);

    return s;
}
