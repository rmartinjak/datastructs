#include <stdlib.h>
#include <check.h>
#include "hashtable.h"

Suite *ht_init_suite(void);
Suite *ht_simple_suite(void);
Suite *ht_args_suite(void);

int main(void)
{
    int number_failed;
    SRunner *sr = srunner_create(NULL);

    srunner_add_suite(sr, ht_init_suite());
    srunner_add_suite(sr, ht_simple_suite());
    srunner_add_suite(sr, ht_args_suite());

    srunner_run_all(sr, CK_NORMAL);

    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
