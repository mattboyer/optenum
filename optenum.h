#ifndef __TABBER_H
#define __TABBER_H


#include "binutils.h"
#include "debug.h"
#include "binary.h"

#include <stdlib.h>


#define _SHORTOPTS "12lv"

#ifdef DEBUG
#define SHORTOPTS _SHORTOPTS "d"
#else
#define SHORTOPTS _SHORTOPTS
#endif

#include "disassemble.h"

// Below this line, we'll be including header files for invidal argument
// parsing modules
#include "parsers/getopt.h"
#ifdef HAVE_GLIB
#include "parsers/glib.h"
#endif

struct option_parsing_function parsers[]={
	getopt_parsers
#ifdef HAVE_GLIB
	glib_parsers
#endif
};


void print_version();
void usage();

#endif
/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
