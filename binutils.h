/* Kludge alert!!
 * bfd.h will cause a preprocessor #error if PACKAGE and PACKAGE_VERSION aren't
 * defined.
 * We'll work around that by temporarily defining then undefining these
 * preprocessor macros
 */
#define PACKAGE
#define PACKAGE_VERSION
#include <bfd.h>
#undef PACKAGE
#undef PACKAGE_VERSION

#include <libiberty.h>
#include <dis-asm.h>

