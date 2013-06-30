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

#ifndef __TABBER_DEBUG_H
#define __TABBER_DEBUG_H

#include "optenum_config.h"

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>

#ifdef DEBUG
bool debug;
#else
#endif




/* TODO Write something about PRIXPTR and why we need it */

#define stringify_constant(constant) stringify_arg(constant)
#define stringify_arg(arg) #arg

#define green(string) "\033[0;32m"string"\033[0;m"
#define red(string) "\033[1;31m"string"\033[0;m"
#define white(string) "\033[1;37m"string"\033[0;m"
#define blue(string) "\033[1;34m"string"\033[0;m"

#ifdef DEBUG

#define debug_output(level, format, ...) \
	fprintf(stderr, level white("[%s:" stringify_constant(__LINE__) "] ") format, __func__,##__VA_ARGS__)

#define debug(...) \
	if (debug) debug_output(green("Debug "), ##__VA_ARGS__)

#define info(...) \
	debug_output(blue("Info "), ##__VA_ARGS__)

#define _error_0()
#define _error_1(format) \
	if (0!=errno) {\
		fprintf(stderr, red("Error ") white("[%s:" stringify_constant(__LINE__) "] ") format, __func__); \
		perror(" "); \
	} else { \
		fprintf(stderr, red("Error ") white("[%s:" stringify_constant(__LINE__) "] ") format "\n", __func__); \
	}

#define _error_2(format, arg1) \
	if (0!=errno) {\
		fprintf(stderr, red("Error ") white("[%s:" stringify_constant(__LINE__) "] ") format, __func__, arg1); \
		perror(" "); \
	} else { \
		fprintf(stderr, red("Error ") white("[%s:" stringify_constant(__LINE__) "] ") format "\n", __func__, arg1); \
	}

#define _error_3(format, arg1, arg2) \
	if (0!=errno) {\
		fprintf(stderr, red("Error ") white("[%s:" stringify_constant(__LINE__) "] ") format, __func__, arg1, arg2); \
		perror(" "); \
	} else { \
		fprintf(stderr, red("Error ") white("[%s:" stringify_constant(__LINE__) "] ") format "\n", __func__, arg1, arg2); \
	}

#else
#define debug(...)
#define info(...)

#define _error_0()
#define _error_1(format) \
	if (0!=errno) {\
		fprintf(stderr, stringify_constant(EXEC_NAME) ": " format); \
		perror(" "); \
	} else { \
		fprintf(stderr, stringify_constant(EXEC_NAME) ": " format "\n"); \
	}

#define _error_2(format, arg1) \
	if (0!=errno) {\
		fprintf(stderr, stringify_constant(EXEC_NAME) ": " format, arg1); \
		perror(" "); \
	} else { \
		fprintf(stderr, stringify_constant(EXEC_NAME) ": " format "\n", arg1); \
	}

#define _error_3(format, arg1, arg2) \
	if (0!=errno) {\
		fprintf(stderr, stringify_constant(EXEC_NAME) ": " format, arg1, arg2); \
		perror(" "); \
	} else { \
		fprintf(stderr, stringify_constant(EXEC_NAME) ": " format "\n", arg1, arg2); \
	}

#endif

#define _error_x(x,A,B,C,FUNC, ...)  FUNC

#define error(...) _error_x(,##__VA_ARGS__,\
	  _error_3(__VA_ARGS__),\
	  _error_2(__VA_ARGS__),\
	  _error_1(__VA_ARGS__),\
	  _error_0(__VA_ARGS__),\
) 

#endif
/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
