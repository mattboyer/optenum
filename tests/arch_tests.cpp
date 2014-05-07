#include <stdlib.h>
#include <check.h>
#include "../arch.c"
#include "check_macros.h"

bfd *test_target = NULL;

void setup (void) {
	bfd_init();
	test_target = bfd_fopen((const char*) "/usr/bin/uname", "default", "r", -1);
	ck_assert_ptr_ne(test_target, NULL);
	ck_assert_str_eq(test_target->filename, "/usr/bin/uname");
	ck_assert_int_ne(test_target->section_count, 0);

	ck_assert_int_ne(test_target->arch_info, 0);
	ck_assert_str_eq(test_target->arch_info->printable_name, "foo");
}

void teardown (void) {
	bfd_close(test_target);
}

START_TEST (test_is_call)
{
  ck_assert_int_eq(parse_call_address(test_target, "callq 0x402460"), 0x402460);
}
END_TEST





Suite *arch_suite (void) {
	Suite *s = suite_create ("Arch");

	/* Core test case */
	TCase *tc_core = tcase_create ("Core");
	tcase_add_checked_fixture (tc_core, setup, teardown);
	tcase_add_test (tc_core, test_is_call);
	suite_add_tcase (s, tc_core);

	return s;
}

int main (void) {
	int number_failed;
	Suite *s = arch_suite();
	SRunner *sr = srunner_create (s);
	srunner_run_all (sr, CK_NORMAL);
	number_failed = srunner_ntests_failed (sr);
	srunner_free (sr);
	return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
