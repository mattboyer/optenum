#ifndef __TABBER_ARCH_H
#define __TABBER_ARCH_H

#include "binutils.h"

#include <stdbool.h>
#include <dlfcn.h>

#include "disassemble.h"


bfd_vma parse_call_address(bfd*, char*);
bool is_call(bfd*, struct disassembly_ring*);
bfd_vma parse_ring_for_call_arg(bfd*, const struct disassembly_ring *, const unsigned int);

#endif
/* vim:set tabstop=4 softtabstop=4 shiftwidth=8 noexpandtab list: */
