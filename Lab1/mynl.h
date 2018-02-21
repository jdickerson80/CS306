#ifndef MYNL_H
#define MYNL_H

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>

typedef enum Style_t
{
	AllLines	= 'a',
	NoLines		= 'n',
	NonEmpty	= 't'
} Style;

/**
 * @brief getUsage gets the default usage of the program
 */
char* getUsage();

/**
 * @brief defaultSeparator
 * @return
 */
char* defaultSeparator();

/**
 * @brief process_stream() is to take an open file stream fpntr, and carry out appropriate
 * actions on it to produce correct output for the file (given the specified options).
 * – You can have any additional parameters you want and any return type you want
 *		(replace the ????’s as desired).
 * – Think about what info needs to be passed from main to this function and whether
 *		any info needs to be passed back to main (in the return value).
 * @param fpntr
 * @param sdf
 */
int process_stream( FILE* fpntr, Style style, char* separator );

/**
 * @brief fgetline is to read and return the next line from the open file stream fpntr.
 * – It is to return the line as a valid C string.
 * – It should return NULL if an I/O error occurs during its execution or if it immediately encounters the file-end.
 * @param fpntr
 * @return
 */
char* fgetline( FILE* fpntr );

#endif // MYNL_H
