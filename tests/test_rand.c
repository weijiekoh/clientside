#include "minunit.h"
#include <stdio.h>

MU_TEST(test_rand) {
    mu_check(1);
}

MU_TEST_SUITE(test_suite) {
	MU_RUN_TEST(test_rand);
}

int main(int argc, char *argv[]) {
	MU_RUN_SUITE(test_suite);
	MU_REPORT();
	return MU_EXIT_CODE;
}
