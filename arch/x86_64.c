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

#include "x86_64.h"

int x86_64__parse_lea(char* inst_line, char **src, char**dst) {
	// This should be changed to LEA
	assert(0==strncmp(inst_line, (const char*) "lea", 3));
	inst_line = &inst_line[3];
	unsigned int arg_idx = 0;

	while( (inst_line[arg_idx] != 0x0) && (inst_line[arg_idx] == ' '))
		arg_idx++;
	// We've only just skipped past the whitespace to the non-whitespace
	// comma-separated operand list
	char *inst_args = &inst_line[arg_idx];
	unsigned int arg_separator_idx = 0;
	unsigned int comment_char_idx = 0;

	char *lea_src_buffer = xmalloc(DISASSEMBLY_BUFFER_LENGTH);
	char *lea_dst_buffer = xmalloc(DISASSEMBLY_BUFFER_LENGTH);
	char *lea_comment_buffer = xmalloc(DISASSEMBLY_BUFFER_LENGTH);

	// TODO couldn't we use something like calloc() instead?
	memset(lea_src_buffer, 0x0, DISASSEMBLY_BUFFER_LENGTH);
	memset(lea_dst_buffer, 0x0, DISASSEMBLY_BUFFER_LENGTH);
	memset(lea_comment_buffer, 0x0, DISASSEMBLY_BUFFER_LENGTH);

	while( (inst_args[arg_separator_idx] != 0x0) && (inst_args[arg_separator_idx] != ','))
		arg_separator_idx++;
	strncpy(lea_src_buffer, inst_args, arg_separator_idx);

	comment_char_idx = arg_separator_idx;
	while( (inst_args[comment_char_idx] != 0x0) && (inst_args[comment_char_idx] != '#'))
		comment_char_idx++;

	// This is wrong *and* ugly
	unsigned int second_arg_end_idx = arg_separator_idx;
	while( (inst_args[second_arg_end_idx] != 0x0) && (inst_args[second_arg_end_idx] != ' '))
		second_arg_end_idx++;

	// This is equally wrong
	unsigned int comment_start_idx = comment_char_idx;
	while( (inst_args[comment_start_idx] != 0x0) && (inst_args[comment_start_idx] != ' '))
		comment_start_idx++;

	strncpy(lea_dst_buffer, &inst_args[arg_separator_idx+1], (second_arg_end_idx - arg_separator_idx - 1));
	strncpy(lea_comment_buffer, &inst_args[comment_start_idx+1], (DISASSEMBLY_BUFFER_LENGTH - (arg_idx + comment_start_idx)));


	info("LEA parsed as:\n\tSRC: %s\n\tDST:-%s-\n\tComment: -%s-\n", lea_src_buffer, lea_dst_buffer, lea_comment_buffer);
	*src = lea_comment_buffer;
	*dst = lea_dst_buffer;

	return 0;
}

int x86_64__parse_mov(char* inst_line, char **src, char**dst) {
	//Do we want the whole line? Yes, I guess we do
	assert(0==strncmp(inst_line, (const char*) "mov", 3));
	inst_line = &inst_line[3];
	unsigned int arg_idx = 0;

	while( (inst_line[arg_idx] != 0x0) && (inst_line[arg_idx] == ' '))
		arg_idx++;

	char *inst_args = &inst_line[arg_idx];
	unsigned int arg_separator_idx = 0;

	char *mov_src_buffer = xmalloc(DISASSEMBLY_BUFFER_LENGTH);
	char *mov_dst_buffer = xmalloc(DISASSEMBLY_BUFFER_LENGTH);

	memset(mov_src_buffer, 0x0, DISASSEMBLY_BUFFER_LENGTH);
	memset(mov_dst_buffer, 0x0, DISASSEMBLY_BUFFER_LENGTH);

	while( (inst_args[arg_separator_idx] != 0x0) && (inst_args[arg_separator_idx] != ','))
		arg_separator_idx++;

	strncpy(mov_src_buffer, inst_args+1, arg_separator_idx);
	strncpy(mov_dst_buffer, &inst_args[arg_separator_idx+1], (DISASSEMBLY_BUFFER_LENGTH - (arg_idx + arg_separator_idx)));

	*src = mov_src_buffer;
	*dst = mov_dst_buffer;
	return 0;
}

bool x86_64__is_call(struct disassembly_ring *instruction) {
	return (0==strncmp(instruction->stream->buffer, (const char*) "call", 4));
}

bfd_vma x86_64__parse_call_address(char* call_line) {
	assert('c'==call_line[0]);
	// As per x86 semantics, there should be one argument right of the call
	uint8_t line_idx = 0;
	uint8_t token_idx = 1;

	char* address_buf = NULL;

	// TODO Why not use strtok????
	for(line_idx=0; line_idx<DISASSEMBLY_BUFFER_LENGTH; ++line_idx) {
		if (0x0==call_line[line_idx])
			break;
		if (' ' == call_line[line_idx]) {
			if (' ' != call_line[line_idx-1])
				++token_idx;
			continue;
		}
		if (2==token_idx)
			break;
	}
	//debug("Address starts at offset %d in line %s\n", line_idx, call_line);

	// address_buf won't necessarily contain an address, it could be
	// something like *%rax, in which case we're screwed
	address_buf = &call_line[line_idx];
	if (address_buf[0]!='0')
		goto bail;

	assert(address_buf[0]=='0');
	assert(address_buf[1]=='x');

	// Can we really trust strtoll???
	// For now, let's roll with it
	bfd_vma callee_address = (bfd_vma) strtoll(address_buf+2, NULL, 16);
	return callee_address;
bail:
	return (bfd_vma) NULL;
}

bfd_vma x86_64__parse_ring_for_call_arg(const struct disassembly_ring *instruction, const unsigned int arg_pos) {

	debug("Looking for value passed to arg %d\n", arg_pos);

	// Save the first call's ring element so we know when we've iterated
	// across the full ring
	const struct disassembly_ring* first_seen = instruction;

	bfd_vma option_descriptor = (bfd_vma) NULL;

	int instruction_offset = 0;
	while (instruction) {

		info("Instruction %d (ring element at %016"PRIXPTR"):\t%s\n",
			instruction_offset, (uintptr_t) instruction,
			instruction->stream->buffer
		);

		// If we go so far backward that we see another function call, the
		// search for argument passing instructions must be called off
		if (instruction != first_seen && x86_64__is_call((struct disassembly_ring*) instruction))
			break;

		// This should be the same irrespective of the instruction used to load
		// a value into the register
		debug("Looking for an instruction loading a value into %s\n", dest_registers[arg_pos]);

		// Argument passing (at least for pointers) is done on x86_64 by
		// loading values into registers.
		//
		// There's a set mapping between the position of the argument in the
		// call and which register it will be loaded into. For our purpose,
		// we'll consider that the relevant argument is loaded by means of the
		// MOV instruction.
		//
		// It is assumed that the option descriptor passed to the option
		// parsing function is a static pointer to a chunk of read-only data in
		// the memory space of the binary - ergo we're looking for a hardcoded
		// value to be the source operand of the MOV
		char *insn_src = NULL;
		char *insn_dst = NULL;
		// TODO Use case instead!!!
		if (0==strncmp(instruction->stream->buffer, (const char*) "mov", 3)) {

			// Read the source and dest of the MOV
			x86_64__parse_mov(instruction->stream->buffer, &insn_src, &insn_dst);
			debug("MOV src:%s\tdst:%s\n", insn_src, insn_dst);
		}
		if (0==strncmp(instruction->stream->buffer, (const char*) "lea", 3)) {
			// Read the source and dest of the LEA
			x86_64__parse_lea(instruction->stream->buffer, &insn_src, &insn_dst);
			debug("LEA src:%s\tdst:%s\n", insn_src, insn_dst);
		}
		if (!(insn_src && insn_dst))
			goto next;

		info("Instruction loaded %s into %s\n", insn_src, insn_dst);
		// We'll handle the fact RDX -> EDX -> DX refer to, essentially, the
		// same register by matching on the last 2 characters of register
		// names.
		// TODO What of the upper/lower 8bit registers, eg. DH / DL?
		if (0==strncmp(&insn_dst[strlen(insn_dst)-2], dest_registers[arg_pos], strlen(dest_registers[arg_pos]))) {
			debug("...which is our register of interest!\n");

			// Bail if the argument of index arg_pos isn't a static address
			// in the memory space of the program
			if (0==strncmp(insn_src, (const char*) "0x", 2)) {
				option_descriptor = (bfd_vma) strtoll(&insn_src[2], NULL, 16);
				info("Option descriptor at %016"PRIXPTR"\n", (uintptr_t) option_descriptor);
			}
		}

		free(insn_src);
		free(insn_dst);


		if (option_descriptor)
			break;

next:
		instruction = instruction->prev;
		if (instruction == first_seen)
			break;
		instruction_offset--;
	}

	info("Arg %d is %016"PRIXPTR"\n", arg_pos, (uintptr_t) option_descriptor);
	return option_descriptor;
}

/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
