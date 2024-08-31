#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>
#include <stdbool.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/ethernet.h>

#include <cmocka.h>

#include "bitter.h"


extern void test_binary_messages(void **state);
extern void test_binary_messages2(void **state);
extern void test_binary_messages3(void **state);
extern void test_binary_messages3_R(void **state);
extern void test_binary_messages_R(void **state);
extern void test_binary_messages_A(void **state);
extern void test_binary_messages_B(void **state);
extern void test_example_1_start_high(void **state);
extern void test_example_1_start_low(void **state);
extern void test_example_2(void **state);


int main(void) {
    // Initialize random number generator.
    srand(time(NULL));

    // Define test suites

    const struct CMUnitTest test_basics[] = {
        cmocka_unit_test(test_binary_messages),
        cmocka_unit_test(test_binary_messages2),
        cmocka_unit_test(test_binary_messages3),
        cmocka_unit_test(test_binary_messages3_R),
        cmocka_unit_test(test_binary_messages_A),
        cmocka_unit_test(test_binary_messages_R),
        cmocka_unit_test(test_binary_messages_B),
        cmocka_unit_test(test_example_1_start_high),
        cmocka_unit_test(test_example_1_start_low),
        cmocka_unit_test(test_example_2),
    };

    // cmocka_set_message_output(CM_OUTPUT_XML);

    int failed_tests = 0;

    printf("\n*** Test bitter functions ***\n\n");
    failed_tests += cmocka_run_group_tests(test_basics, NULL, NULL);

    printf("\nTotal failed tests: %s%d%s\n\n",
        (failed_tests == 0 ? "\033[32m" : "\033[31m"),
        failed_tests,
        "\033[0m");
    return failed_tests;
}
