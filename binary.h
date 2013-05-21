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
