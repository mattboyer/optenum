#include <stdlib.h>
#include <check.h>
#include "../disassemble.c"

#define ck_assert_int_gt(X, Y) _ck_assert_int(X, >, Y)
/* Pointer comparsion macros with improved output compared to ck_assert(). */
/* OP may only be == or !=  */
#define _ck_assert_ptr(X, OP, Y) do { \
  void* _ck_x = (X); \
  void* _ck_y = (Y); \
  ck_assert_msg(_ck_x OP _ck_y, "Assertion '"#X#OP#Y"' failed: "#X"==%p, "#Y"==%p", _ck_x, _ck_y); \
} while (0)
#define ck_assert_ptr_eq(X, Y) _ck_assert_ptr(X, ==, Y)
#define ck_assert_ptr_ne(X, Y) _ck_assert_ptr(X, !=, Y)

void setup (void) {
	return;
}

void teardown (void) {
	return;
}

START_TEST (test_append_to_empty)
{
	struct parsed_option_list *empty = NULL;
	ck_assert_ptr_eq(empty, NULL);
	empty = append_option(empty, "foo", true, ONE_DASH);
	ck_assert_ptr_ne(empty, NULL);
	ck_assert_ptr_eq(empty->prev, NULL);
	ck_assert_ptr_eq(empty->next, NULL);
	ck_assert_ptr_ne(empty->option, NULL);

	ck_assert_str_eq(empty->option->name, "foo");
	ck_assert_ptr_eq(empty->option->description, NULL);
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

	ck_assert_ptr_ne(options, NULL);
	ck_assert_ptr_eq(options->prev, option_one);
	ck_assert_ptr_eq(options->next, NULL);
	ck_assert_ptr_ne(options->option, NULL);

	ck_assert_str_eq(options->option->name, "bar");
	ck_assert_ptr_eq(options->option->description, NULL);
	ck_assert_int_eq(options->option->takes_argument, false);
	ck_assert_int_eq(options->option->type, TWO_DASH);

	ck_assert_str_eq(options->prev->option->name, "foo");
	ck_assert_ptr_eq(options->prev->option->description, NULL);
	ck_assert_int_eq(options->prev->option->takes_argument, true);
	ck_assert_int_eq(options->prev->option->type, ONE_DASH);
	ck_assert_ptr_eq(options->prev->next, options);
	ck_assert_ptr_eq(options->prev->prev, NULL);
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

START_TEST (test_concatenate_to_empty)
{
	struct parsed_option_list *right = NULL;
	right = append_option(right, "foo", true, ONE_DASH);
	right = append_option(right, "bar", true, ONE_DASH);
	right = append_option(right, "baz", false, TWO_DASH);

	ck_assert_int_gt(right, NULL);

	struct parsed_option_list *left = NULL;
	concatenate_parsed_options(&left, right);

	ck_assert_int_gt(left, NULL);
	ck_assert_str_eq(left->option->name, "foo");
	ck_assert_str_eq(left->next->option->name, "bar");
	ck_assert_str_eq(left->next->next->option->name, "baz");
	ck_assert_int_eq(left->next->next->next, NULL);
	ck_assert_int_eq(left->prev, NULL);
}
END_TEST

START_TEST (test_concatenate_to_nonempty)
{
	struct parsed_option_list *right = NULL;
	right = append_option(right, "foo", true, ONE_DASH);
	right = append_option(right, "bar", true, ONE_DASH);
	right = append_option(right, "baz", false, TWO_DASH);

	ck_assert_int_gt(right, NULL);

	struct parsed_option_list *left = NULL;
	left = append_option(left, "alpha", true, ONE_DASH);
	left = append_option(left, "bravo", true, ONE_DASH);
	left = append_option(left, "charlie", false, TWO_DASH);

	struct parsed_option_list *old_left = left;
	concatenate_parsed_options(&left, right);
	// That's really weird. It would be better to change the left list to be
	// the head of the concatenated list
	ck_assert_int_eq(left, old_left);

	ck_assert_int_gt(left, NULL);
	ck_assert_str_eq(left->option->name, "charlie");

	ck_assert_str_eq(left->prev->option->name, "bravo");
	ck_assert_str_eq(left->prev->prev->option->name, "alpha");

	ck_assert_str_eq(left->next->option->name, "foo");
	ck_assert_str_eq(left->next->next->option->name, "bar");
	ck_assert_str_eq(left->next->next->next->option->name, "baz");

	ck_assert_int_eq(left->next->next->next->next, NULL);
	ck_assert_int_eq(left->prev->prev->prev, NULL);
}
END_TEST


START_TEST (test_free_options_appended)
{
	struct parsed_option_list *options = NULL;
	options = append_option(options, "foo", true, ONE_DASH);

	struct parsed_option_list *option_one= options;
	options = append_option(options, "bar", false, TWO_DASH);
	struct parsed_option_list *option_two= options;
	options = append_option(options, "baz", true, NO_DASH);

	ck_assert_int_gt(options, NULL);
	ck_assert_int_eq(options->prev, option_two);
	ck_assert_str_eq(options->prev->prev, option_one);

	ck_assert_int_gt(options->option->name, NULL);
	ck_assert_int_gt(options->prev->option->name, NULL);
	ck_assert_int_gt(options->prev->prev->option->name, NULL);

	free_parsed_options(options);
	ck_assert_int_gt(options, NULL);
	ck_assert_int_eq(options->prev, option_two);
	ck_assert_str_eq(options->prev->prev, option_one);
	// How can we assert that the calls to free() have been successfull?
}
END_TEST


Suite *arch_suite (void) {
	Suite *s = suite_create ("Options");

	TCase *tc_core = tcase_create ("Appending");
	tcase_add_checked_fixture (tc_core, setup, teardown);
	tcase_add_test (tc_core, test_append_to_empty);
	tcase_add_test (tc_core, test_append_two);
	tcase_add_test (tc_core, test_append_three);
	suite_add_tcase (s, tc_core);

	TCase *tc_append = tcase_create ("Concatenating");
	tcase_add_checked_fixture (tc_core, setup, teardown);
	tcase_add_test (tc_core, test_concatenate_to_empty);
	tcase_add_test (tc_core, test_concatenate_to_nonempty);
	suite_add_tcase (s, tc_append);

	TCase *tc_free = tcase_create ("Freeing");
	tcase_add_checked_fixture (tc_core, setup, teardown);
	tcase_add_test (tc_core, test_free_options_appended);
	suite_add_tcase (s, tc_free);

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
