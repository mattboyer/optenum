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

#include "optenum.h"


void destroy_parser_list(struct option_parsing_function_list *head) {
	while(head) {
		struct option_parsing_function_list *next= head->next;
		debug("Freeing parser list element at %016"PRIXPTR"\n", (uintptr_t) head);
		free(head);
		head = next;
	}
}

struct option_parsing_function_list *append_option_parser(struct option_parsing_function_list *head, struct option_parsing_function *meta) {

	struct option_parsing_function_list *last = NULL;
	struct option_parsing_function_list *first = head;
	while (head) {
		last = head;
		head = head->next;
	}

	struct option_parsing_function_list *new_parser = (struct option_parsing_function_list *) xcalloc(1, sizeof(struct option_parsing_function_list));

	new_parser->function = meta;
	debug("Appending %s to supported parsers in list elt %016"PRIXPTR"\n", meta->name, (uintptr_t) new_parser);
	if (last)
		last->next = new_parser;
	if (!first)
		first = new_parser;
	return first;
}

struct option_parsing_function_list *prepare_parser_list() {
	unsigned int num_parsers = (int) sizeof(parsers)/sizeof(struct option_parsing_function);
	info("array of parsers has %u parsers\n", num_parsers);
	unsigned int parser_idx = 0;

	struct option_parsing_function_list *parser_list = NULL;

	while(num_parsers > parser_idx) {
		parser_list = append_option_parser(parser_list, &parsers[parser_idx]);
		parser_idx++;
	}
	return parser_list;
}

void print_parsers(struct option_parsing_function_list *head) {
	while (head) {
		fprintf(stdout, "%s\n", head->function->name);
		head = head->next;
	}
}

void print_version() {
	printf(stringify_constant(EXEC_NAME) " v%d.%02d.%s\n", VERSION_MAJOR,
		VERSION_MINOR, stringify_constant(VERSION_CHANGESET));
}

void usage() {
	printf("Usage: "stringify_constant(EXEC_NAME) " [-" SHORTOPTS "] <file>\n");
}

int main(int argc, char** argv) {

	bfd* binary_bfd = NULL;

	asymbol **dynamic_symbols = NULL;
	size_t num_dynamic_symbols = 0;

#ifdef DEBUG
	debug=false;
#endif

	struct option_parsing_function_list *supported_functions = NULL;
	supported_functions = prepare_parser_list();
	assert(supported_functions);

	debug("Supported function list starts at %016"PRIXPTR"\n", (uintptr_t) supported_functions);

	/* Parse options and argument */
	enum OPTION_TYPE parsed_option_display_filter = (enum OPTION_TYPE) NO_TYPE;
	int option=0;
	opterr = 0;
	while ((option = getopt(argc, argv, SHORTOPTS)) != -1) {
		switch(option) {
			case 'v':
				print_version();
				destroy_parser_list(supported_functions);
				return EXIT_SUCCESS;
				break;

			case 'l':
				print_parsers(supported_functions);
				destroy_parser_list(supported_functions);
				return EXIT_SUCCESS;
				break;
#ifdef DEBUG
			case 'd':
				debug=true;
				break;
#endif
			case '1':
				parsed_option_display_filter = ONE_DASH;
				break;

			case '2':
				parsed_option_display_filter = TWO_DASH;
				break;
			case '?':
				destroy_parser_list(supported_functions);
				usage();
				return EXIT_FAILURE;
				break;
		};
	};


	// There needs to be ONE argument after the options
	if ((argc-optind) != 1) {
		usage();
		destroy_parser_list(supported_functions);
		return EXIT_FAILURE;
	}


	/////////////////////////////
	bfd_init();


	debug("Attempting to open %s\n", argv[optind]);
	if (!is_valid_file(argv[optind])) {
		destroy_parser_list(supported_functions);
		return EXIT_FAILURE;
	}

	binary_bfd = bfd_fopen(argv[optind], "default", "r", -1);
	info("Operating on %08" PRIXPTR "\n", (uintptr_t) binary_bfd);

	unsigned int is_correct_format = bfd_check_format(binary_bfd, bfd_object);
	info("format %d\n", is_correct_format);

	if (0==is_correct_format) {
		error("Could not open %s: %s", argv[optind], bfd_errmsg(bfd_get_error()));
		goto FAIL;
	}

	// Parse dynamic symbols in the target binary
	num_dynamic_symbols = parse_binary_dynamic_symbols(binary_bfd, &dynamic_symbols);
	if (0>=num_dynamic_symbols) {
		error("No dynamic symbols found in %s", argv[optind]);
		goto FAIL;
	}

	// Parse the dynamic relocations in the target binary and make a note of
	// any relocs to one of the supported argument parsing functions
	unsigned int interesting_relocs = parse_dynamic_relocs(binary_bfd, dynamic_symbols, supported_functions);
	info("%u relocs found in binary\n", interesting_relocs);
	free(dynamic_symbols);


	struct parsed_option_list *options = NULL;

	if (0<interesting_relocs) {

		struct option_parsing_function_list *parser_iter = supported_functions;
		while(parser_iter) {
			if (!parser_iter->reloc)
				goto next_match;

			info("Looking for calls to:\t%s:\t%016"PRIXPTR"\n", parser_iter->function->name, parser_iter->reloc);
			parser_iter->call_address = get_reloc_call_address(binary_bfd, parser_iter->reloc);
			// This will parse all code sections for calls to this parser
			filter_code_sections(binary_bfd, parser_iter, &options);
next_match:
			parser_iter = parser_iter->next;
		}
	} else {
		error("Couldn't find any relocs to interesting external function calls in %s", argv[optind]);
		goto FAIL;
	}

	display_options(options, parsed_option_display_filter);
	free_parsed_options(options);

	bfd_close(binary_bfd);
	destroy_parser_list(supported_functions);
	return EXIT_SUCCESS;
FAIL:
	bfd_close(binary_bfd);
	destroy_parser_list(supported_functions);
	return EXIT_FAILURE;
}

/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
