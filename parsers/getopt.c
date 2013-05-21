#include "getopt.h"

struct parsed_option_list *iterate_short_opts(bfd *binary_bfd, const bfd_vma shortopts) {
	asection *shortopt_section = find_vma_section(binary_bfd, shortopts);

	// Prepare a list of options for this call
	struct parsed_option_list *options_found = NULL;

	size_t sec_size = bfd_get_section_size(shortopt_section);
	bfd_byte *section_data = (bfd_byte *) xmalloc (sec_size);
	bfd_get_section_contents(binary_bfd, shortopt_section, section_data, 0, sec_size);
	debug("Loading the contents of section %s (%ld bytes)\n", shortopt_section->name, sec_size);

	size_t shortopt_offset = (shortopts - shortopt_section->vma);
	info("shortopts live in section %s at offset %ld\n", shortopt_section->name, shortopt_offset);


	bfd_byte *optstring = &section_data[shortopt_offset];
	info("Short opts %s\n", optstring);

	unsigned int opt_idx = 0;
	while (0x0 != optstring[opt_idx]) {

		char opt_char[2] = {optstring[opt_idx], 0x0};

		bool has_arg = false;
		// Does this option require an argument?
		if (':' == optstring[opt_idx+1]) {
			has_arg = false;
			opt_idx++;
		}
		options_found = append_option(options_found, opt_char, has_arg, ONE_DASH);
		opt_idx++;
	}

	free(section_data);
	return options_found;
}

struct parsed_option_list *iterate_long_opts(bfd *binary_bfd, const bfd_vma longopts) {

	asection *longopt_section = find_vma_section(binary_bfd, longopts);

	// Prepare a list of options for this call
	struct parsed_option_list *options_found = NULL;

	size_t sec_size = bfd_get_section_size(longopt_section);
	bfd_byte *section_data = (bfd_byte *) xmalloc (sec_size);
	bfd_get_section_contents(binary_bfd, longopt_section, section_data, 0, sec_size);
	debug("Loading the contents of section %s (%ld bytes)\n", longopt_section->name, sec_size);

	size_t longopt_offset = (longopts - longopt_section->vma);
	info("longopts live in section %s at offset %ld\n", longopt_section->name, longopt_offset);

	char *all_zeroes = alloca(sizeof(struct option));
	memset(all_zeroes, 0x0, sizeof(struct option));

	while (longopt_offset) {

		/* As per the man page for getopt_long(3), the last element in
		 * the array of struct option elements has to be all zeroes
		 */
		if (0==memcmp(&section_data[longopt_offset], all_zeroes, sizeof(struct option)))
			break;

		struct option *long_option = (struct option*) &section_data[longopt_offset];

		debug("struct option @ section offset %ld\t loaded into %016" PRIXPTR "\n", longopt_offset, (uintptr_t) long_option);

		// XXX assume the option string lives in the same section. that's bullshit of course
		// XXX Also, casting long_option-> which is a member of a struct in the local arch to a bfd_vma is super dodgy
		size_t name_offset = ( (bfd_vma)long_option->name - longopt_section->vma);


		debug("option name @ %016" PRIXPTR ": %s\n", (uintptr_t) long_option->name, &section_data[name_offset]);

		options_found = append_option(options_found, (const char*) &section_data[name_offset], (bool) (1==long_option->has_arg), TWO_DASH);



		longopt_offset += sizeof(struct option);

	}

	debug("Last struct option in array at offset %ld vma %016" PRIXPTR "\n", longopt_offset, longopt_section->vma+longopt_offset);
	free(section_data);

	return options_found;
}

/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
