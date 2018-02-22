#include "mynl.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

//Macros
#define MAX_LINE_LENGTH ( 1000 )
#define NON_LINE_NUMBER_PADDING ( "          " )

// Initialize all of the static variables
static char usage[] = "The usage for the program is:\n"
					  "./mynl [-bSTYLE] [-sSTRING] [FILE...]\n"
					  "Values for STYLE:\n\t– a meaning number all lines\n\t– n meaning number no lines\n\t– t meaning number only nonempty lines\n"
					  "Default STYLE if -b not provided: t\n"
					  "Default STRING if not provided is: tab ('\\t’)\n";

static char line_buff[ MAX_LINE_LENGTH ];
static char DefaultSeparator[] = "\t";
static unsigned int lineNumberWidth = 6;
static char const* lineNumberFormat = "%*" PRIdMAX "%s";
static unsigned int lineCounter = 1;

const char* const getUsage()
{
	return usage;
}

char* defaultSeparator()
{
	return DefaultSeparator;
}

int process_stream( FILE* fpntr, Style style, char* separator )
{
	char* line;

	// switch on the specified style
	switch ( style )
	{
	case AllLines:
		while ( ( line = fgetline( fpntr ) ) != NULL )
		{
			printf( lineNumberFormat, lineNumberWidth, lineCounter, separator );
			printf("%s", line );
			lineCounter++;
		}
		break;

	case NoLines:
		while ( ( line = fgetline( fpntr ) ) != NULL )
		{
			printf("%s%s", NON_LINE_NUMBER_PADDING, line );
		}
		break;

	case NonEmpty:
		while ( ( line = fgetline( fpntr ) ) != NULL )
		{
			// if the line is empty
			if ( strlen( line ) == 1 )
			{
				// print the appropriate line
				printf("%s%s", NON_LINE_NUMBER_PADDING, "\n" );
			}
			else // line is not empty, so print line number
			{
				printf( lineNumberFormat, lineNumberWidth, lineCounter, separator );
				printf("%s", line );
				lineCounter++;
			}
		}
		break;
	// default case to catch user's garbage input...
	default:
		fprintf( stderr, "Incorrect style arguement given.\n%s", getUsage() );
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

char* fgetline( FILE* fpntr )
{
	// init variables
	size_t temp = 0;
	int next = 0;

	// get all of the characters
	while ( ( next = fgetc( fpntr ) ) != '\n' && next != EOF )
	{
		line_buff[ temp++ ] = next;
	}

	// catch the end of file
	if ( next == EOF )
	{
		return NULL;
	}

	// found the end of the line, so format to make a correct string
	line_buff[ temp++ ] = '\n';
	line_buff[ temp++ ] = '\0';

	// return line buffer
	return line_buff;
}
