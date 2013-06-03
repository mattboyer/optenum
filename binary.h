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

#ifndef __TABBER_BINARY_H
#define __TABBER_BINARY_H

#include "debug.h" //needed for debug stuff

#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h> // XXX Needed for strcmp. Shouldn't be here!!!!

#include "binutils.h"

#include "disassemble.h"
#include "arch.h"

bool break_on_first;

// Prototypes go here
size_t parse_binary_dynamic_symbols(bfd *, asymbol ***);
unsigned int parse_dynamic_relocs(bfd *, asymbol **, struct option_parsing_function_list*);
bfd_vma get_reloc_call_address(bfd *, bfd_vma);

void filter_code_sections(bfd *, struct option_parsing_function_list*, struct parsed_option_list**);
void filter_section_for_call(bfd *, asection *, struct option_parsing_function_list*, struct parsed_option_list**);
asection *find_vma_section(bfd *, bfd_vma);
bool is_valid_file(const char*);

#endif
/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
