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

#include "arch.h"

/*
We need to implement a dispatcher for every function defined in arch.h. The 
dispatcher will look up the correct architecture-specific function in the
optenum executable and call it, passing the arguments it was given.

We'll use macro expansion in order not to replicate that code for every
function. This means we need a way to make the prototype of the function to
dispatch a parameter to the macro. This is done by passing the following
arguments to the dispatcher macro:

ARCH_DISPATCH(func_name, return_type, arg1_type, arg2_type, ..., argN_type)

Since no name is given to the macro, we'll need arbitrary names for the
arguments taken by the expanded dispatcher. We'll use TYPES_NAMES for (arg_type,
arg_name) pairs and NAMES for the names themselves. These macros are taken from
http://stackoverflow.com/a/8814003

*/

#define TYPES_NAMES_0()
#define TYPES_NAMES_1(A)                    A alpha
#define TYPES_NAMES_2(A,B)                  A alpha, B beta
#define TYPES_NAMES_3(A,B,C)                A alpha, B beta, C charlie
#define TYPES_NAMES_4(A,B,C,D)              A alpha, B beta, C charlie, D delta

#define TYPES_NAMES_X(x,A,B,C,D,FUNC, ...)  FUNC  

#define TYPES_NAMES(...)                    TYPES_NAMES_X(,##__VA_ARGS__,\
                                          TYPES_NAMES_4(__VA_ARGS__),\
                                          TYPES_NAMES_3(__VA_ARGS__),\
                                          TYPES_NAMES_2(__VA_ARGS__),\
                                          TYPES_NAMES_1(__VA_ARGS__),\
                                          TYPES_NAMES_0(__VA_ARGS__)\
                                         ) 


#define NAMES_0()
#define NAMES_1(A)                    alpha
#define NAMES_2(A,B)                  alpha, beta
#define NAMES_3(A,B,C)                alpha, beta, charlie
#define NAMES_4(A,B,C,D)              alpha, beta, charlie, delta




#define NAMES_X(x,A,B,C,D,FUNC, ...)  FUNC  

#define NAMES(...)                    NAMES_X(,##__VA_ARGS__,\
                                          NAMES_4(__VA_ARGS__),\
                                          NAMES_3(__VA_ARGS__),\
                                          NAMES_2(__VA_ARGS__),\
                                          NAMES_1(__VA_ARGS__),\
                                          NAMES_0(__VA_ARGS__)\
                                         ) 


#define ARCH_DISPATCH(func_name, return_type, ...) \
	return_type func_name(bfd *target_binary, TYPES_NAMES(__VA_ARGS__)) {	\
		\
		if (func_name##_ptr) {	\
			debug("Using cached pointer for %s: %016"PRIXPTR"\n", #func_name,	\
				(uintptr_t) func_name##_ptr);\
			return func_name##_ptr(NAMES(__VA_ARGS__));	\
		}	\
		\
		void *self_dl_handle = dlopen(NULL, RTLD_LAZY);	\
		debug("Target compiled for %s\n",	\
			target_binary->arch_info->printable_name);	\
		return_type (*arch_func_ptr)(__VA_ARGS__);	\
		const char *prefix = NULL;	\
		\
		if (0==strcmp("i386:x86-64", target_binary->arch_info->printable_name))	\
			prefix = X86_64_PREFIX;	\
		if (0==strcmp("i386", target_binary->arch_info->printable_name))	\
			prefix = X86_PREFIX;	\
		\
		if (!prefix) {	\
			error("Can't find function %s for architecture %s", #func_name,	\
				target_binary->arch_info->printable_name);\
		}	\
		char *arch_func_name = malloc(200*sizeof(char));\
		strcpy(arch_func_name, prefix);\
		strcat(arch_func_name, #func_name);\
		info("Attempting to resolve %s\n", arch_func_name);\
		*(void**) (&arch_func_ptr) = dlsym(self_dl_handle, arch_func_name);	\
		if (!arch_func_ptr) {	\
			error("dlsym() failed: %s", dlerror());	\
		}	\
		info("Using %s at %016"PRIXPTR"\n", arch_func_name,	\
			(uintptr_t) arch_func_ptr);\
		func_name##_ptr = arch_func_ptr;\
		return_type return_value = arch_func_ptr(NAMES(__VA_ARGS__));	\
		dlclose(self_dl_handle);	\
		free(arch_func_name);\
		return return_value;	\
	}


static const char* X86_64_PREFIX = "x86_64__";
static const char* X86_PREFIX = "i386__";

// We'll use these to cache pointers to the arch-specific functions after the
// first successfull dlsym(3) lookup
static bfd_vma (*parse_call_address_ptr)(char*) = NULL;
static bool (*is_call_ptr)(struct disassembly_ring*) = NULL;
static bfd_vma (*parse_ring_for_call_arg_ptr)(const struct disassembly_ring*,
	const unsigned int) = NULL;

// Expand dipatchers for each function declared in arch.h
ARCH_DISPATCH(parse_call_address, bfd_vma, char *)
ARCH_DISPATCH(is_call, bool, struct disassembly_ring *)
ARCH_DISPATCH(parse_ring_for_call_arg, bfd_vma, const struct disassembly_ring *,
	const unsigned int)

/* vim:set tabstop=4 softtabstop=4 shiftwidth=4 noexpandtab : */
