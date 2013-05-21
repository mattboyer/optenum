/* Operations performed on the binary go in here */
#include "binary.h"

static const char* skip_sections[] = {".init", ".fini", NULL};

bool is_valid_file(const char* file_name) {
	assert(file_name);
	struct stat file_props;


	if (0!=access(file_name, F_OK)) {
		error("Couldn't access(2) %s", file_name);
		return false;
	}

	// Is it a regular file?
	if (0!=stat(file_name, &file_props)) {
		error("Couldn't stat(2) %s", file_name);
		return false;
	}

	if (!S_ISREG(file_props.st_mode)) {
		error("%s is not a regular file", file_name);
		return false;
	}

	return true;
}

size_t parse_binary_dynamic_symbols(bfd *binary_bfd, asymbol ***storage_address) {
	long symbol_storage_length = 0;
	size_t num_symbols = 0;
	asymbol** symbol_storage = NULL;

	assert(binary_bfd);

	/* This will process dynamic symbols only. That's all we want, since
	 * option parsing will be done by libc/glib/some other external library */
	symbol_storage_length = bfd_get_dynamic_symtab_upper_bound(binary_bfd);
	assert(symbol_storage_length > 0);
	debug("Symbol storage reqd %ld\n", symbol_storage_length);

	symbol_storage = (asymbol**) xmalloc(symbol_storage_length);

	num_symbols = bfd_canonicalize_dynamic_symtab(binary_bfd, symbol_storage);
	assert(num_symbols > 0);
	info("Number of symbols copied %ld\n", num_symbols);

#ifdef DEBUG
	unsigned int symbol_idx = 0;
	for(symbol_idx = 0; symbol_idx < num_symbols; ++symbol_idx) {
		asymbol* sym = symbol_storage[symbol_idx];
		debug("Symbol stored locally at %016" PRIXPTR ":\n\tname %s\n\tflags %04X\n\tvalue %016" PRIXPTR "\n\tunion: %016" PRIXPTR "\n\tin section %016" PRIXPTR " (%s)\n", (uintptr_t) sym, sym->name, sym->flags, (uintptr_t) sym->value, (uintptr_t) &sym->udata, (uintptr_t) sym->section, sym->section->name);

	}
#endif

	//XXX Freeing symbol_storage will have to be done by the caller!!!
	*storage_address = symbol_storage;
	return num_symbols;
}

unsigned int parse_dynamic_relocs(bfd *binary_bfd, asymbol **symbols, struct option_parsing_function_list *parsers) {

	unsigned int relocs_found = 0;

	unsigned int relocation_storage_length = 0;
	unsigned int num_relocs = 0;
	arelent** relocation_storage = NULL;

	unsigned int rel_idx = 0;

	relocation_storage_length = bfd_get_dynamic_reloc_upper_bound(binary_bfd);
	info("%d bytes needed to store relocation information for this binary\n", relocation_storage_length);


	// So it's looking like the array of arelent is allocated during the
	// call whereas the symbols have to be allocated beforehand?! Would be
	// nice to have some documentation, eh?
	// WRROOONNNG!!!! It needs to be allocated here!!!!
	relocation_storage = (arelent**) xmalloc(relocation_storage_length); // not section_specific

	// The 3rd arg HAS to be the dynamic symtab we've already populated
	num_relocs = bfd_canonicalize_dynamic_reloc(binary_bfd, relocation_storage, symbols);
	info("Copied %d relocs\n", num_relocs);

	// Right... this spike is getting interesting now... parse the relocation entries
	for(rel_idx = 0; rel_idx < num_relocs; ++rel_idx) {
		asymbol* relocated_symbol = *relocation_storage[rel_idx]->sym_ptr_ptr;

		debug("rel at address %016" PRIXPTR "\n", (uintptr_t) relocation_storage[rel_idx]);
		debug("\tfor symbol %s at %016" PRIXPTR "\n", relocated_symbol->name, (uintptr_t) relocated_symbol);
		debug("\taddress: %016" PRIXPTR "\n", (uintptr_t) relocation_storage[rel_idx]->address);
		debug("\taddend: %016" PRIXPTR "\n", (uintptr_t) relocation_storage[rel_idx]->addend);

		// Compare this reloc against all supported functions
		struct option_parsing_function_list *parser_iter = parsers;
		while (parser_iter) {
			debug("Comparing %s to supported function %s\n", relocated_symbol->name, parser_iter->function->name);
			if (0==strcmp(parser_iter->function->name, relocated_symbol->name)) {
				info("Found reloc for %s in binary\n", parser_iter->function->name);
				parser_iter->reloc = relocation_storage[rel_idx]->address;
				relocs_found++;
				break;
			}
			parser_iter = parser_iter->next;
		}
	}
	free(relocation_storage);

	return relocs_found;
}

bfd_vma get_reloc_call_address(bfd *binary_bfd, bfd_vma reloc) {

	asection *reloc_section = find_vma_section(binary_bfd, reloc);
	if (!reloc_section) {
		error("Couldn't find section for %016"PRIXPTR, reloc);
		return (bfd_vma) NULL;
	}

	assert(reloc_section);

	unsigned long section_offset = reloc - reloc_section->vma;
	//debug("Section \"%s\"'s vma is %08" PRIXPTR " offset of %08" PRIXPTR "\n", reloc_section->name, (uintptr_t) reloc_section->vma, (uintptr_t) section_offset);

	// Read the whole section, even though we're only going to copy one
	// teeny-tiny bfd_vma from it
	// TODO This could be made more efficient
	bfd_byte *section_buffer = NULL;
	bfd_malloc_and_get_section(binary_bfd, reloc_section, &section_buffer);

	// Copy the reloc's jump address
	bfd_vma call_address = (bfd_vma) NULL;

	memcpy(&call_address, section_buffer+section_offset, sizeof(bfd_vma));
	info("Call address: %016" PRIXPTR " (what will be called!)\n", (uintptr_t) call_address);
	// TODO Alignment is a problem - the actual argument given to call (on
	// x86) may be a few bytes before the jump address
	free(section_buffer);
	return call_address;
}

void filter_code_sections(bfd *binary_bfd, struct option_parsing_function_list *parser, struct parsed_option_list **options) {
	asection *section = NULL;
	section = binary_bfd->sections;
	assert(section);
	while (section) {
		if (!(section->flags & SEC_CODE))
			goto next;

		unsigned int skip_section_idx = 0;
		while (skip_sections[skip_section_idx]) {
			if (0==strcmp(section->name, skip_sections[skip_section_idx])) {
				info("Skipping section %s\n", skip_sections[skip_section_idx]);
				goto next;
			}
			++skip_section_idx;
		}

		debug("Section name %s flags %08X has code\n", section->name, section->flags);
		filter_section_for_call(binary_bfd, section, parser, options);
next:
		section = section->next;
	}
}

void filter_section_for_call(bfd *binary_bfd, asection *code_section, struct option_parsing_function_list *parser, struct parsed_option_list **options) {

	bool call_found = false;

	// Read the contents of code_section into a buffer
	size_t sec_size = bfd_get_section_size(code_section);
	bfd_byte *section_data = (bfd_byte *) xmalloc (sec_size);
	bfd_get_section_contents(binary_bfd, code_section, section_data, 0, sec_size);
	info("Disassembling the contents of section %s (%ld bytes)\n", code_section->name, sec_size);

	// Prepare a ring of file-like buffers into which we'll write the last
	// N instructions disassembled from the section
	struct disassembly_ring *instruction_ring = prepare_ring(DISASSEMBLY_RING_LENGTH);

	// We let libopcodes' disassembler() decide what disassembly function
	// to use based on the binary
	disassembler_ftype disasser = disassembler(binary_bfd);

	// We need to prepare a struct disassemble_info - this will control the
	// content of the disassembled text printed out by the disassemler
	// function pointed to by disasser above
	struct disassemble_info *dinfo = (struct disassemble_info *) xmalloc(sizeof(struct disassemble_info));

	// OK, here's the thing. There is a type named fprintf_ftype for the
	// function pointer we're going to use in our struct disassemble_info.
	// We'll be passing the struct's 'stream' member as its first argument
	// but here's the thing: it doesn't HAVE to work on FILE's. Anything is
	// good as long as its consistent!
	init_disassemble_info(dinfo, instruction_ring->stream, (fprintf_ftype) buf_fprintf);


	dinfo->bytes_per_line = 0;
	dinfo->bytes_per_chunk = 0;

	dinfo->buffer = section_data;
	dinfo->buffer_vma = code_section->vma;
	dinfo->buffer_length = sec_size;
	dinfo->section = code_section;

	size_t bytes_disassd = 0;
	while (bytes_disassd<sec_size) {
		// Use a fresh buffer for the purpose of retrieving the
		// disassembled code
		instruction_ring->stream->position=0;
		dinfo->stream = instruction_ring->stream;

#ifdef DEBUG
		// Save the VMA of the next instruction to decode for debugging
		// purposes
		bfd_vma code_address = code_section->vma + bytes_disassd;
#endif

		// ...then decode it
		bytes_disassd += disasser(code_section->vma + bytes_disassd, dinfo);

		debug("Disassembled %ld/%ld bytes of section %s into %016"
			PRIXPTR ":%s\n", bytes_disassd, sec_size,
			code_section->name, (uintptr_t) instruction_ring,
			instruction_ring->stream->buffer
		);

		if (is_call(binary_bfd, instruction_ring)) {
			bfd_vma callee_address = parse_call_address(binary_bfd, instruction_ring->stream->buffer);

			if ((bfd_vma) NULL==callee_address)
				continue;

			debug("Call at %016" PRIXPTR " to VMA %016" PRIXPTR "\n",
				(uintptr_t) code_address, (uintptr_t) callee_address
			);

			// TODO The way we handle alignment here needs to be looked at
			// TODO It should probably be done upstream
			if (callee_address == ((parser->call_address >>4)<<4 )) {
				info("Candidate function call found in section %s at VMA %016" PRIXPTR "\n", code_section->name, (uintptr_t) code_address);
				call_found = true;

				// Extract the value passed to the argument of the option
				// parsing function specified in parser->function. It is
				// expected that the value we load is a pointer to a static
				// chunk of data in the target binary that describes all
				// options known to the option parsing function.
				if (parser->function->short_option_parser) {
					bfd_vma descriptor = parse_ring_for_call_arg(binary_bfd, instruction_ring, parser->function->short_option_descriptor_pos);

					struct parsed_option_list *short_options = parser->function->short_option_parser(binary_bfd, descriptor);
					debug("Short option list %016"PRIXPTR"\n", (uintptr_t) short_options);
					concatenate_parsed_options(options, short_options);
				}

				if (parser->function->long_option_parser) {
					bfd_vma descriptor = parse_ring_for_call_arg(binary_bfd, instruction_ring, parser->function->long_option_descriptor_pos);
					struct parsed_option_list *long_options = parser->function->long_option_parser(binary_bfd, descriptor);

					debug("Long option list %016"PRIXPTR"\n", (uintptr_t) long_options);
					concatenate_parsed_options(options, long_options);
				}
			}
		}

		// Rotate the ring
		instruction_ring = instruction_ring->next;
	}

	if (!call_found)
		info("Didn't find any candidates in section %s\n", code_section->name);

	destroy_ring(instruction_ring);

	free(dinfo);
	free(section_data);
}

asection *find_vma_section(bfd *binary_bfd, bfd_vma vma_to_locate) {
	assert(binary_bfd);

	asection *section_iterator = binary_bfd->sections;
	while (section_iterator) {
		if ((vma_to_locate > section_iterator->vma) && (vma_to_locate <= (section_iterator->vma + section_iterator->size)))
			break;

		section_iterator = section_iterator->next;
	}
	
	if (section_iterator)
		debug("VMA %016" PRIXPTR " is in section %s\n", 
			(uintptr_t) vma_to_locate, section_iterator->name);
	return section_iterator;
}

/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
