#include "../debug.h" //needed for debug stuff

#include <stdlib.h>
#include <string.h>

#include "../binutils.h"

// Provides binary access functions
#include "../binary.h"
#include "../disassemble.h"

// This header file is specific to the function being parsed
#include <getopt.h>

struct parsed_option_list *iterate_long_opts(bfd *, const bfd_vma);
struct parsed_option_list *iterate_short_opts(bfd *, const bfd_vma);


// Register the argument parsing functions handled here
#define getopt_parsers\
	{"getopt_long", iterate_short_opts, 3, iterate_long_opts, 4}, \
	{"getopt", iterate_short_opts, 3, NULL, 0}, \
	{"getopt_long_only",  iterate_long_opts, 4, NULL, 0},


/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
