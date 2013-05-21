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


#define _error(format, ...) \
	if (0!=errno) {\
		fprintf(stderr, red("Error ") white("[%s:" stringify_constant(__LINE__) "] ") format, __func__, ##__VA_ARGS__); \
		perror(" "); \
	} else { \
		fprintf(stderr, red("Error ") white("[%s:" stringify_constant(__LINE__) "] ") format "\n", __func__, ##__VA_ARGS__); \
	}

#define error(...) _error(__VA_ARGS__)

#else
#define debug(...) false
#define info(...) false

#define error(format, ...) \
	if (0!=errno) {\
		fprintf(stderr, stringify_constant(EXEC_NAME) ": " format, ##__VA_ARGS__); \
		perror(" "); \
	} else { \
		fprintf(stderr, stringify_constant(EXEC_NAME) ": " format "\n", ##__VA_ARGS__); \
	}
#endif

#endif
/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
