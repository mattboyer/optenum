/* Disassembly-related code */
#include "disassemble.h"

struct disassembly_ring *prepare_ring(unsigned int instructions) {
	unsigned int instruction_index = 0;

	struct disassembly_ring *first = NULL;
	struct disassembly_ring *prev = NULL;

	struct disassembly_ring *instruction = NULL;

	for(instruction_index=0; instruction_index < instructions; ++instruction_index) {
		instruction = (struct disassembly_ring *) xcalloc(1, sizeof(struct disassembly_ring));
		debug("Creating an instruction ring element at %016"PRIXPTR"\n", (uintptr_t) instruction);

		struct disassembly_pseudo_file *disassembly_stream = (struct disassembly_pseudo_file*) xcalloc(1, sizeof(struct disassembly_pseudo_file));
		disassembly_stream->buffer = (char*) xcalloc(DISASSEMBLY_BUFFER_LENGTH, sizeof(char));
		disassembly_stream->position = 0;
		instruction->stream = disassembly_stream;



		if (0==instruction_index) {
			first = instruction;
		} else {
			instruction->prev = prev;
			instruction->prev->next = instruction;
		}

		prev = instruction;
	}
	instruction->next = first;
	first->prev = instruction;

	return first;
}

void destroy_ring(struct disassembly_ring *head) {
	struct disassembly_ring* first_seen = head;
	while (head) {
		debug("Freeing ring element at %016"PRIXPTR"\n", (uintptr_t) head);
		struct disassembly_ring* next = head->next;

		free(head->stream->buffer);
		free(head->stream);
		free(head);

		if (next == first_seen)
			break;
		head = next;
	}
}

void print_ring(struct disassembly_ring *head) {
	struct disassembly_ring* first_seen = head;
	while (head) {
		info("Instruction ring element at %016"PRIXPTR"\n\t%s\n", (uintptr_t) head, head->stream->buffer);
		head = head->prev;
		if (head == first_seen)
			break;
	}
}


struct parsed_option_list *append_option(struct parsed_option_list *head, const char *name, bool has_arg, enum OPTION_TYPE type) {
	struct parsed_option *new_option = (struct parsed_option*) xcalloc(1, sizeof(struct parsed_option));

	// We need to copy the name into a new buffer
	char *option_name = (char *) xcalloc(OPTION_NAME_LENGTH, sizeof(char));
	strncpy(option_name, name, (size_t) OPTION_NAME_LENGTH);

	new_option->name = option_name;

	//FIXME
	new_option->description = NULL;
	new_option->takes_argument = has_arg;
	new_option->type = type;

	struct parsed_option_list *new_option_list = (struct parsed_option_list*) xcalloc(1, sizeof(struct parsed_option_list));
	new_option_list->option = new_option;
	new_option_list->prev = NULL;
	new_option_list->next = NULL;

	// Get to the end of the list
	while (head && head->next)
		head = head->next;

	if (head) {
		head->next = new_option_list;
		new_option_list->prev = head;
		debug("Appended new parsed option after %016"PRIXPTR"\n", (uintptr_t) head);
	} else {
		head = new_option_list;
	}

	return new_option_list;
}

void concatenate_parsed_options(struct parsed_option_list **left, struct parsed_option_list *right) {
	debug("left %016"PRIXPTR" (pointing to %016"PRIXPTR") right %016"PRIXPTR"\n", (uintptr_t) left, (uintptr_t) *left, (uintptr_t) right);

	if (!right)
		return;

	// Jump to the start of the right-hand list
	while (right->prev)
		right = right->prev;

	if (NULL==*left) {
		// If there's nothing on the left, make it the right-hand list
		*left=right;
	} else {
		// ...else jump to the end of the left-hand list
		struct parsed_option_list *last_left = *left;
		while (last_left->next) {
			last_left = last_left->next;
		}
		// ...and link
		last_left->next = right;
		last_left->next->prev = last_left;
	}
}

void free_parsed_options(struct parsed_option_list *options) {
	while (options->prev)
		options=options->prev;

	while (options) {
		free(options->option->name);
		free(options->option);
		struct parsed_option_list *next = options->next;
		free(options);
		options = next;
	}
}

void display_options(const struct parsed_option_list *options, enum OPTION_TYPE type) {

	while (options->prev)
		options=options->prev;

	while (options) {
		debug("Options %016"PRIXPTR"\n", (uintptr_t) options);
		debug("Option %016"PRIXPTR"\n", (uintptr_t) options->option);
		debug("Option name %016"PRIXPTR"\n", (uintptr_t) options->option->name);

		if ((type==NO_TYPE)||(type==options->option->type)) {
			switch(options->option->type) {
				case ONE_DASH:
					fprintf(stdout, "-%s\n", options->option->name);
					break;

				case TWO_DASH:
					fprintf(stdout, "--%s\n", options->option->name);
					break;
				default:
					break;
			}
		}
		options = options->next;
	}
}



int buf_fprintf(struct disassembly_pseudo_file* output_stream, const char *format, ...) {
	// This function may be called several times for any given call to the
	// disassembly function. We need to replicate the behaviour of fprintf
	// while printing to a single buffer.
	va_list args;
	int num_chars_printed=0;

	//debug("call to buf_fprintf pos=%d buffer=%016" PRIXPTR "\n", output_stream->position, (uintptr_t) output_stream->buffer);

	// Clear the buffer if the position has been reset
	if (0==output_stream->position)
		memset(output_stream->buffer, 0x0, DISASSEMBLY_BUFFER_LENGTH);

	va_start (args, format);
	num_chars_printed = vsnprintf (&output_stream->buffer[output_stream->position], (DISASSEMBLY_BUFFER_LENGTH - output_stream->position), format, args);
	va_end (args);
	if (0 > num_chars_printed) {
		error("Couldn't write disassembled instruction token into %016" PRIXPTR, (uintptr_t) output_stream->position);
		return -1;
	}

	output_stream->position+=num_chars_printed;
	return num_chars_printed;
}



/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab list: */
