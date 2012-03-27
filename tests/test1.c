#include <stdlib.h>
#include <check.h>
#include "hashtable.h"

START_TEST (test_ht_init_no_hash_no_cmp)
{
    hashtable *ht;
    
    ht_init(&ht, NULL, NULL);

    fail_unless(ht == NULL);
}
END_TEST

Suite *ht_init_suite(void)
{
    Suite *s = suite_create("ht");
    TCase *tc_ht_init = tcase_create("ht_init");
    tcase_add_test(tc_ht_init, test_ht_init_no_hash_no_cmp);
    suite_add_tcase(s, tc_ht_init);

    return s;
}

int main(void)
{
    int number_failed;
    Suite *s = ht_init_suite();
    SRunner *sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);

    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
