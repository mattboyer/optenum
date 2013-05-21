cmake_minimum_required (VERSION 2.6)
project (OPTENUM)
string(TOLOWER ${PROJECT_NAME} EXEC_NAME)
set (VERSION_MAJOR 0)
set (VERSION_MINOR 10)

# Make the Hg revision part of the the version information baked
# into optenum_config.h
execute_process (	COMMAND hg heads --template {rev}
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
			OUTPUT_VARIABLE HG_REVISION
		)
set (VERSION_CHANGESET ${HG_REVISION})

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

find_library (GLIB "libglib-2.0.so")
iF (GLIB)
	list( APPEND ${PROJECT_NAME}_SRCS parsers/glib.c)
endiF (GLIB)



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

# vim:set tabstop=8 softtabstop=8 shiftwidth=8 noexpandtab list:
