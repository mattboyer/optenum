#include <stdlib.h>
#include <check.h>
#include "../disassemble.c"

void setup (void) {
	return;
}

void teardown (void) {
	return;
}

START_TEST (test_append_to_empty)
{
	struct parsed_option_list *empty = NULL;
	ck_assert_int_eq(empty, NULL);
	empty = append_option(empty, "foo", true, ONE_DASH);
	ck_assert_int_gt(empty, NULL);
	ck_assert_int_eq(empty->prev, NULL);
	ck_assert_int_eq(empty->next, NULL);
	ck_assert_int_gt(empty->option, NULL);

	ck_assert_str_eq(empty->option->name, "foo");
	ck_assert_int_eq(empty->option->description, NULL);
	ck_assert_int_eq(empty->option->takes_argument, true);
	ck_assert_int_eq(empty->option->type, ONE_DASH);
}
END_TEST

START_TEST (test_append_two)
{
	struct parsed_option_list *options = NULL;
	options = append_option(options, "foo", true, ONE_DASH);

	struct parsed_option_list *option_one= options;
	options = append_option(options, "bar", false, TWO_DASH);

	ck_assert_int_gt(options, NULL);
	ck_assert_int_eq(options->prev, option_one);
	ck_assert_int_eq(options->next, NULL);
	ck_assert_int_gt(options->option, NULL);

	ck_assert_str_eq(options->option->name, "bar");
	ck_assert_int_eq(options->option->description, NULL);
	ck_assert_int_eq(options->option->takes_argument, false);
	ck_assert_int_eq(options->option->type, TWO_DASH);

	ck_assert_str_eq(options->prev->option->name, "foo");
	ck_assert_int_eq(options->prev->option->description, NULL);
	ck_assert_int_eq(options->prev->option->takes_argument, true);
	ck_assert_int_eq(options->prev->option->type, ONE_DASH);
	ck_assert_int_eq(options->prev->next, options);
	ck_assert_int_eq(options->prev->prev, NULL);
}
END_TEST

START_TEST (test_append_three)
{
	struct parsed_option_list *options = NULL;
	options = append_option(options, "foo", true, ONE_DASH);

	struct parsed_option_list *option_one= options;
	options = append_option(options, "bar", false, TWO_DASH);
	struct parsed_option_list *option_two= options;
	options = append_option(options, "baz", true, NO_DASH);

	ck_assert_int_gt(options, NULL);

	ck_assert_int_eq(options->prev, option_two);
	ck_assert_int_eq(options->next, NULL);
	ck_assert_int_eq(options->prev->next, options);
	ck_assert_int_eq(options->prev->prev, option_one);
	ck_assert_int_eq(options->prev->prev->next, option_two);
	ck_assert_int_eq(options->prev->prev->next->next, options);
	ck_assert_int_eq(options->prev->prev->prev, NULL);

	ck_assert_str_eq(options->option->name, "baz");
	ck_assert_int_eq(options->option->description, NULL);
	ck_assert_int_eq(options->option->takes_argument, true);
	ck_assert_int_eq(options->option->type, NO_DASH);

	ck_assert_str_eq(options->prev->option->name, "bar");
	ck_assert_int_eq(options->prev->option->description, NULL);
	ck_assert_int_eq(options->prev->option->takes_argument, false);
	ck_assert_int_eq(options->prev->option->type, TWO_DASH);

}
END_TEST


Suite *arch_suite (void) {
	Suite *s = suite_create ("Options");

	/* Core test case */
	TCase *tc_core = tcase_create ("Core");
	tcase_add_checked_fixture (tc_core, setup, teardown);
	tcase_add_test (tc_core, test_append_to_empty);
	tcase_add_test (tc_core, test_append_two);
	tcase_add_test (tc_core, test_append_three);
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
