#ifndef MYNL_H
#define MYNL_H

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

typedef enum StyleTypes_t
{
	AllLines	= 'a',
	NoLines		= 'n',
	NonEmpty	= 't'
} StyleTypes;

typedef enum CommandLineArguements_t
{
	Style	= 'b',
	String	= 's'
} CommandLineArguements;

/**
 * @brief getUsage gets the default usage of the program
 */
const char* const getUsage();

/**
 * @brief defaultSeparator
 * @return
 */
char* defaultSeparator();

/**
 * @brief process_stream() is to take an open file stream fpntr, and carry out appropriate
 * actions on it to produce correct output for the file (given the specified options).
 * @param fpntr
 * @param style
 * @param separator
 * @return
 */
int process_stream( FILE* fpntr, StyleTypes styleTypes, char* separator );

/**
 * @brief fgetline is to read and return the next line from the open file stream fpntr.
 * – It is to return the line as a valid C string.
 * – It should return NULL if an I/O error occurs during its execution or if it immediately encounters the file-end.
 * @param fpntr
 * @return
 */
char* fgetline( FILE* fpntr );

#endif // MYNL_H
