#include "../debug.h" //needed for debug stuff

#include "../binutils.h"

// Provides binary access functions
#include "../binary.h"
#include "../disassemble.h"

struct parsed_option_list *g_option_magic(bfd *, const bfd_vma);

#define glib_parsers\
	{"g_option_context_parse", NULL, 0, g_option_magic, 0}, 
