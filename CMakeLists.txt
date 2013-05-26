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



file(GLOB ${PROJECT_NAME}_SRCS RELATIVE ${PROJECT_SOURCE_DIR}
	*.c
	# The following are argument parsing modules
	parsers/getopt.c

	arch/*.c
)

find_package (PkgConfig)
pkg_check_modules (GLIB2 glib-2.0)

iF (GLIB2_FOUND)
	message(STATUS "Building parser for GLib's g_option_*")
	include_directories (${GLIB2_INCLUDE_DIRS})

	list( APPEND ${PROJECT_NAME}_SRCS parsers/glib.c)
endiF (GLIB2_FOUND)



find_library (LIBIBERTY "iberty")
find_library (LIBBFD "bfd")
find_library (LIBOPCODES "opcodes")
find_library (ZLIB "z")
find_library (DL "dl")

message(STATUS "Linking against libiberty file: ${LIBIBERTY}")
message(STATUS "Linking against libbfd file: ${LIBBFD}")
message(STATUS "Linking against libopcodes file: ${LIBOPCODES}")


add_executable (${EXEC_NAME} ${${PROJECT_NAME}_SRCS})

if (CMAKE_COMPILER_IS_GNUCC)
	message (STATUS "GCC detected - making compiler pedantic")
	set_target_properties (${EXEC_NAME} PROPERTIES COMPILE_FLAGS "-std=c99 -Wall -pedantic")
endif (CMAKE_COMPILER_IS_GNUCC)

# libbfd requires libdl and libz
target_link_libraries (${EXEC_NAME} bfd iberty opcodes z dl)

if (${CMAKE_BUILD_TYPE} STREQUAL "RELEASE")
	message (STATUS "Compiling a RELEASE build - enabling install rules")
	install(TARGETS ${EXEC_NAME} RUNTIME DESTINATION "bin")
else (${CMAKE_BUILD_TYPE} STREQUAL "RELEASE")
	message (WARNING "Install rules are only available for DEBUG builds.")
endif (${CMAKE_BUILD_TYPE} STREQUAL "RELEASE")

# vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab list:
