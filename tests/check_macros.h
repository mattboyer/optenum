#ifndef CHECK_MAJOR_VERSION
#error "check not found"
#endif

#if CHECK_MICRO_VERSION < 10

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

#endif
