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

#ifndef __OPTENUM_DISASSEMBLE_H
#define __OPTENUM_DISASSEMBLE_H



#include "debug.h" //needed for debug stuff
//#include "binary.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "binutils.h"


#define DISASSEMBLY_BUFFER_LENGTH 200
#define DISASSEMBLY_RING_LENGTH 8

#define OPTION_NAME_LENGTH 50

struct disassembly_pseudo_file {
	char *buffer;
	unsigned int position;
};

struct disassembly_ring {
	struct disassembly_pseudo_file *stream;
	struct disassembly_ring *next;
	struct disassembly_ring *prev;
};


// This typedef is for the function that will inspect the disassembly ring and populate output option structures
typedef struct parsed_option_list *(*arg_parser) (bfd *, const bfd_vma);


struct option_parsing_function {
	const char* name;
	arg_parser short_option_parser;
	const unsigned int short_option_descriptor_pos;
	arg_parser long_option_parser;
	const unsigned int long_option_descriptor_pos;
};

struct option_parsing_function_list {
	struct option_parsing_function *function;
	bfd_vma reloc;
	bfd_vma call_address;
	struct option_parsing_function_list *next;
};

enum OPTION_TYPE {
	ONE_DASH,
	TWO_DASH,
	NO_DASH,
	NO_TYPE,
};

struct parsed_option {
	char *name;
	char *description;
	bool takes_argument;
	enum OPTION_TYPE type;
};

struct parsed_option_list {
	struct parsed_option *option;
	struct parsed_option_list *prev;
	struct parsed_option_list *next;
};

struct parsed_option_list *append_option(struct parsed_option_list *, const char *, bool, enum OPTION_TYPE);
void concatenate_parsed_options(struct parsed_option_list **, struct parsed_option_list *);
void free_parsed_options(struct parsed_option_list *);
void display_options(const struct parsed_option_list *, enum OPTION_TYPE);


struct disassembly_ring *prepare_ring(unsigned int);
void destroy_ring(struct disassembly_ring *);
void print_ring(struct disassembly_ring *);

int buf_fprintf(struct disassembly_pseudo_file *, const char *, ...);


#endif
/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
