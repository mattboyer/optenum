#include "x86_64.h"

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

	strncpy(mov_src_buffer, inst_args, arg_separator_idx);
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

	bfd_vma option_descriptor = NULL;

	int instruction_offset = 0;
	while (instruction) {

		info("Instruction %d (ring element at %016"PRIXPTR"):\t%s\n",
			instruction_offset, (uintptr_t) instruction,
			instruction->stream->buffer
		);

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
		if (0==strncmp(instruction->stream->buffer, (const char*) "mov", 3)) {

			// Read the source and dest of the MOV
			char *mov_src = NULL;
			char *mov_dst = NULL;
			x86_64__parse_mov(instruction->stream->buffer, &mov_src, &mov_dst);
			debug("MOV src:%s\tdst:%s\n", mov_src, mov_dst);

			debug("Looking for a MOV into %s\n", dest_registers[arg_pos]);

			if (0==strncmp(mov_dst, dest_registers[arg_pos], strlen(dest_registers[arg_pos]))) {
				assert(0==strncmp(mov_src, (const char*) "$0x", 3));
				option_descriptor = (bfd_vma) strtoll(&mov_src[3], NULL, 16);
				info("Option descriptor at %016"PRIXPTR"\n", (uintptr_t) option_descriptor);
			}

			free(mov_src);
			free(mov_dst);
		}

		if (option_descriptor)
			break;


		instruction = instruction->prev;
		if (instruction == first_seen)
			break;
		instruction_offset--;
	}

	info("Arg %d is %016"PRIXPTR"\n", arg_pos, (uintptr_t) option_descriptor);
	return option_descriptor;
}

/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
