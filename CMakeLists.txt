# Copyright (C) 2013 Matt Boyer.
# All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the project nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

cmake_minimum_required (VERSION 2.6)
project (OPTENUM)
string(TOLOWER ${PROJECT_NAME} EXEC_NAME)
set (VERSION_MAJOR 0)
set (VERSION_MINOR 10)

# Make the Git revision part of the the version information baked
# into optenum_config.h
execute_process (	COMMAND git log --format=%h -n1
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_REVISION
		)
set (VERSION_CHANGESET ${GIT_REVISION})

configure_file ("${PROJECT_SOURCE_DIR}/optenum_config.h.in"
    "${PROJECT_BINARY_DIR}/optenum_config.h")

include_directories (${PROJECT_BINARY_DIR})

set_property (DIRECTORY . PROPERTY COMPILE_DEFINITIONS_DEBUG "DEBUG")
#add_subdirectory("parsers")

if (NOT CMAKE_BUILD_TYPE)
	message (STATUS "Build type not specified - defaulting to RELEASE build")
	set (CMAKE_BUILD_TYPE "RELEASE")
endif (NOT CMAKE_BUILD_TYPE)


file(GLOB ${PROJECT_NAME}_SRCS RELATIVE ${PROJECT_SOURCE_DIR}
	*.c
	# The following are argument parsing modules
	parsers/getopt.c

	arch/*.c
)

find_package (PkgConfig)

# Some distros have separate '*-dev' packages for header files, dev docs and
# the like. Consequently, a pkc-config .pc file may be found on the system for
# eg. popt without the requisite header files actually being present.
pkg_check_modules (GLIB2 glib-2.0)
if (GLIB2_FOUND)
	find_file(GLIB2_HEADER "glib.h" ${GLIB2_INCLUDE_DIRS})
	if (GLIB2_HEADER)
		message(STATUS "Building parser for GLib's g_option_* - header"
			" at ${GLIB2_HEADER}")
		include_directories (${GLIB2_INCLUDE_DIRS})
		list (APPEND ${PROJECT_NAME}_SRCS parsers/glib.c)
		file (APPEND "${PROJECT_BINARY_DIR}/optenum_config.h"
			"#define HAVE_GLIB2\n"
		)
	else (GLIB2_HEADER)
		message(WARNING "glib2 is installed but its header files"
			" couldn't be found")
	endif (GLIB2_HEADER)
endif (GLIB2_FOUND)

pkg_check_modules (POPT popt)
if (POPT_FOUND)
	find_file(POPT_HEADER "popt.h")
	if (POPT_HEADER)
		message(STATUS "Building parser for popt's poptGetContext - "
			"header at ${POPT_HEADER}")
		include_directories (${POPT_INCLUDE_DIRS})
		list (APPEND ${PROJECT_NAME}_SRCS parsers/popt.c)
		file (APPEND "${PROJECT_BINARY_DIR}/optenum_config.h"
			"#define HAVE_POPT\n"
		)
	else (POPT_HEADER)
		message(WARNING "popt is installed but its header files"
			" couldn't be found")
	endif (POPT_HEADER)
endif (POPT_FOUND)

find_library(LIBCHECK "check")
find_file(CHECK_HEADER "check.h")
if (LIBCHECK AND CHECK_HEADER)

	if (${CMAKE_BUILD_TYPE} STREQUAL "DEBUG")
		message (STATUS "libcheck found and DEBUG build - enabling unit tests")

		file (MAKE_DIRECTORY "${PROJECT_BINARY_DIR}/tests")
		file (GLOB ${PROJECT_NAME}_TEST_SRCS RELATIVE "${PROJECT_SOURCE_DIR}/tests"
			tests/*.c
		)

		foreach (test_source ${${PROJECT_NAME}_TEST_SRCS})
			string (REPLACE ".c" "" test_executable ${test_source})
			add_executable ("tests/${test_executable}" "tests/${test_source}")
			target_link_libraries ("tests/${test_executable}" bfd iberty opcodes check ${CMAKE_DL_LIBS})
		endforeach (test_source ${${PROJECT_NAME}_TEST_SRCS})

	endif (${CMAKE_BUILD_TYPE} STREQUAL "DEBUG")
else (LIBCHECK AND CHECK_HEADER)
	message (STATUS "libcheck not found - no unit tests")
endif (LIBCHECK AND CHECK_HEADER)

find_file(LIBBFD_HEADER "bfd.h")
if (NOT LIBBFD_HEADER)
	message(FATAL_ERROR "libbfd header not found on this system")
endif (NOT LIBBFD_HEADER)

#
# If at all possible, we want the executable to link against shared objects for
# libbfd and libopcodes.
# Unfortunately, just passing '-lbfd -lopcodes' to the compiler at linking time
# tends to result in a huge executable statically linked against libbfd.a and
# libopcodes.a
#
# This is further complicated by the fact that some distros ship a
# libbfd.so/libopcodes.so file that is *not* an actual shared object, but
# rather a linker script pointing to their respective .a archive.
#
# This means we cannot just give "libbfd.so" as argument to CMake's
# find_library, we'll have to dynamically discover whether there is an "actual"
# shared object with a versioned filename (eg. "libbfd-2.25.so") and give this
# to find_library() with an option to fall back on the .so file in the hope
# that it's a symlink to a real shared object and failing that, the .a
# archive.
#
# At any rate, the variable set by find_library() is then used to set the
# IMPORTED_LOCATION of a CMake "imported" target for that third-party library.
# This ensures the right path is given to the compiler in the make rule that
# links the executable.
#
function (find_binutils_dep libname)
	foreach (library_prefix ${CMAKE_SYSTEM_PREFIX_PATH})
		foreach (library_dir "lib64" "lib")
			file (GLOB ${libname}_so_path RELATIVE "${library_prefix}/${library_dir}" "${library_prefix}/${library_dir}/lib${libname}-*.so")
			if (${libname}_so_path)
				break ()
			endif (${libname}_so_path)
		endforeach (library_dir)
		if (${libname}_so_path)
			break ()
		endif (${libname}_so_path)
	endforeach (library_prefix)

	find_library (${libname}_path ${${libname}_so_path} NAMES "lib${libname}.so" "lib${libname}.a")
	if (NOT ${libname}_path)
		message(FATAL_ERROR "lib${libname} library not found on this system")
	endif (NOT ${libname}_path)

	add_library ("binutils_dep_${libname}" SHARED IMPORTED)
	set_target_properties ("binutils_dep_${libname}" PROPERTIES IMPORTED_LOCATION ${${libname}_path})
endfunction (find_binutils_dep)

find_binutils_dep (bfd)
find_binutils_dep (opcodes)

find_file(LIBIBERTY_HEADER "libiberty.h" PATHS /usr/include /usr/include/libiberty)
if (NOT LIBIBERTY_HEADER)
	message(FATAL_ERROR "libiberty header not found on this system")
endif (NOT LIBIBERTY_HEADER)

# Travis CI has CMake version 2.8.7 so we'll use "PATH" instead of "DIRECTORY"
get_filename_component(LIBIBERTY_INCLUDES ${LIBIBERTY_HEADER} "PATH")
include_directories (${LIBIBERTY_INCLUDES})


message(STATUS "Linking against libbfd:\n\tLib: ${bfd_path}\n\tHeader: ${LIBBFD_HEADER}")
message(STATUS "Linking against libopcodes:\n\tLib: ${opcodes_path}")
message(STATUS "Using libiberty header: ${LIBIBERTY_HEADER}")

if (EXISTS ${LIBIBERTY_INCLUDES}/ansidecl.h)
	set (ANSIDECL_SOURCE ${LIBIBERTY_INCLUDES}/ansidecl.h)
else (EXISTS ${LIBIBERTY_INCLUDES}/ansidecl.h)
	set (ANSIDECL_SOURCE ansidecl.h)
endif (EXISTS ${LIBIBERTY_INCLUDES}/ansidecl.h)
configure_file ("${PROJECT_SOURCE_DIR}/binutils.h.in"
    "${PROJECT_BINARY_DIR}/binutils.h")


add_executable (${EXEC_NAME} ${${PROJECT_NAME}_SRCS})
target_link_libraries (${EXEC_NAME} binutils_dep_bfd binutils_dep_opcodes ${CMAKE_DL_LIBS})

if (CMAKE_COMPILER_IS_GNUCC)
	message (STATUS "GCC detected - making compiler pedantic")
	set_target_properties (${EXEC_NAME} PROPERTIES COMPILE_FLAGS
		"-std=c99 -Wall -pedantic"
	)
endif (CMAKE_COMPILER_IS_GNUCC)




if (${CMAKE_BUILD_TYPE} STREQUAL "RELEASE")
	message (STATUS "Compiling a RELEASE build - enabling install rules")
	install(TARGETS ${EXEC_NAME} RUNTIME DESTINATION "bin")
	install(FILES "${EXEC_NAME}.sh" DESTINATION "share/optenum")
	install(FILES "doc/${EXEC_NAME}.1" DESTINATION "share/man/man1")
else (${CMAKE_BUILD_TYPE} STREQUAL "RELEASE")
	message (STATUS "Install rules are only available for RELEASE builds")
endif (${CMAKE_BUILD_TYPE} STREQUAL "RELEASE")

#enable_testing()
#add_test(NAME foo COMMAND "/usr/bin/false")

# vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab :
