#ifndef __TABBER_ARCH_X86_64_H
#define __TABBER_ARCH_X86_64_H

#include "../binutils.h"
#include "../disassemble.h"


bfd_vma x86_64__parse_call_address(char*);

int x86_64__parse_mov(char*, char **, char **);
bool x86_64__is_call(struct disassembly_ring *);
bfd_vma x86_64__parse_ring_for_call_arg(const struct disassembly_ring *, const unsigned int);

// How should we handle the case where RDX -> EDX -> DX -> DH, DL refer to,
// essentially, the same register?
static const char *dest_registers[] = {"", "%rdi", "%rsi", "%edx", "%ecx", "%r8", "%r9"};

#endif
/* vim:set tabstop=4 softtabstop=4 shiftwidth=8 noexpandtab list: */
