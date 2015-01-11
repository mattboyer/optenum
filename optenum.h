/*
Copyright (C) 2013 Matt Boyer.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of the project nor the names of its contributors
   may be used to endorse or promote products derived from this software
   without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
SUCH DAMAGE.
*/

#ifndef __OPTENUM_H
#define __OPTENUM_H


#include "binutils.h"
#include "debug.h"
#include "binary.h"

#include <stdlib.h>


#define _SHORTOPTS "12blv"

#ifdef DEBUG
#define SHORTOPTS _SHORTOPTS "d"
#else
#define SHORTOPTS _SHORTOPTS
#endif

#include "disassemble.h"

// Below this line, we'll be including header files for invidal argument
// parsing modules
#include "parsers/getopt.h"
#ifdef HAVE_GLIB2
#include "parsers/glib.h"
#endif
#ifdef HAVE_POPT
#include "parsers/popt.h"
#endif

struct option_parsing_function parsers[]={
	getopt_parsers
#ifdef HAVE_GLIB2
	glib_parsers
#endif
#ifdef HAVE_POPT
	popt_parsers
#endif
};


void print_version();
void usage();

#endif
/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
