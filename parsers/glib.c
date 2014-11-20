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

#include "glib.h"

struct parsed_option_list *parse_goption_main_entries(bfd *binary_bfd, const bfd_vma option_entries) {

	asection *option_entry_section = find_vma_section(binary_bfd, option_entries);

	// Prepare a list of options for this call
	struct parsed_option_list *options_found = NULL;

	size_t sec_size = bfd_get_section_size(option_entry_section);
	bfd_byte *section_data = (bfd_byte *) xmalloc (sec_size);
	bfd_get_section_contents(binary_bfd, option_entry_section, section_data, 0, sec_size);
	debug("Loading the contents of section %s (%ld bytes)\n", option_entry_section->name, sec_size);

	size_t option_entry_offset = (option_entries - option_entry_section->vma);
	info("option_entries live in section %s at offset %ld\n", option_entry_section->name, option_entry_offset);


	while ((bfd_vma) 0!= section_data[option_entry_offset]) {

		GOptionEntry *glib_option = (GOptionEntry*) &section_data[option_entry_offset];
		info("GOptionEntry at %016"PRIXPTR"\n", (uintptr_t) glib_option);

		bfd_byte *long_name_section_data = NULL;
		asection *long_name_section = find_vma_section(binary_bfd, (bfd_vma) glib_option->long_name);
		if (!long_name_section)
			break;
		debug("option name lives in section %s\n", long_name_section->name);
		if (long_name_section == option_entry_section) {
			long_name_section_data = section_data;
		} else {
			size_t long_name_sec_size = bfd_get_section_size(long_name_section);
			long_name_section_data = (bfd_byte *) xmalloc (long_name_sec_size);
			bfd_get_section_contents(binary_bfd, long_name_section, long_name_section_data, 0, long_name_sec_size);
		}
		size_t long_option_offset = ((bfd_vma) glib_option->long_name - long_name_section->vma);

		info("long name %s\n", &long_name_section_data[long_option_offset]);
		options_found = append_option(options_found, (const char*) &long_name_section_data[long_option_offset], (bool) false, TWO_DASH);

		if ((char) 0x0 != glib_option->short_name)
			options_found = append_option(options_found, (const char*) &glib_option->short_name, (bool) false, ONE_DASH);

		option_entry_offset += sizeof(GOptionEntry);
	}

	free(section_data);

	return options_found;
}

/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
